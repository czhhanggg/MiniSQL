#include "buffer/buffer_pool_manager.h"

#include "glog/logging.h"
#include "page/bitmap_page.h"

static const char EMPTY_PAGE_DATA[PAGE_SIZE] = {0};

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager) {
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size_);
  for (size_t i = 0; i < pool_size_; i++) {
    free_list_.emplace_back(static_cast<frame_id_t>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  for (auto page : page_table_) {
    FlushPage(page.first);
  }
  delete[] pages_;
  delete replacer_;
}

/**
 * Fetch the requested page from the buffer pool.
 *
 * 1.  Search the page table for the requested page (P).
 * 1.1 If P exists, pin it and return it immediately.
 * 1.2 If P does not exist, find a replacement page (R) from either the free list or the replacer.
 *     Note that pages are always found from the free list first.
 * 2.  If R is dirty, write it back to the disk.
 * 3.  Delete R from the page table and insert P.
 * 4.  Update P's metadata, read in the page content from disk, and then return a pointer to P.
 */
Page *BufferPoolManager::FetchPage(page_id_t page_id) {
  lock_guard<recursive_mutex> lock(latch_);

  // 1. Search the page table for the requested page
  auto it = page_table_.find(page_id);
  if (it != page_table_.end()) {
    // 1.1 Page exists in buffer pool — pin it and return
    frame_id_t frame_id = it->second;
    Page *page = &pages_[frame_id];
    page->pin_count_++;
    replacer_->Pin(frame_id);
    return page;
  }

  // 1.2 Page not in buffer pool — find a replacement frame
  frame_id_t frame_id = TryToFindFreePage();
  if (frame_id == INVALID_FRAME_ID) {
    return nullptr;  // All frames are pinned, cannot fetch
  }

  Page *page = &pages_[frame_id];

  // 2. If the replacement page (R) is dirty, write it back to disk
  if (page->IsDirty()) {
    disk_manager_->WritePage(page->page_id_, page->GetData());
    page->is_dirty_ = false;
  }

  // 3. Delete R from the page table (if it had a valid page)
  if (page->page_id_ != INVALID_PAGE_ID) {
    page_table_.erase(page->page_id_);
  }

  // Insert P into the page table
  page_table_[page_id] = frame_id;

  // 4. Update P's metadata, read page content from disk
  page->page_id_ = page_id;
  page->pin_count_ = 1;
  page->is_dirty_ = false;
  disk_manager_->ReadPage(page_id, page->GetData());

  return page;
}

/**
 * Create a new page in the buffer pool.
 *
 * 0.  Make sure you call AllocatePage!
 * 1.  If all the pages in the buffer pool are pinned, return nullptr.
 * 2.  Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
 * 3.  Update P's metadata, zero out memory and add P to the page table.
 * 4.  Set the page ID output parameter. Return a pointer to P.
 */
Page *BufferPoolManager::NewPage(page_id_t &page_id) {
  lock_guard<recursive_mutex> lock(latch_);

  // 0. Allocate a new page from disk (logical page ID)
  page_id = AllocatePage();

  // 1. If all pages in the buffer pool are pinned, return nullptr
  frame_id_t frame_id = TryToFindFreePage();
  if (frame_id == INVALID_FRAME_ID) {
    // Cannot find a free frame — all pages are pinned
    return nullptr;
  }

  Page *page = &pages_[frame_id];

  // 2. If the victim page is dirty, write it back to disk before eviction
  if (page->IsDirty()) {
    disk_manager_->WritePage(page->page_id_, page->GetData());
    page->is_dirty_ = false;
  }

  // Remove old page mapping from the page table
  if (page->page_id_ != INVALID_PAGE_ID) {
    page_table_.erase(page->page_id_);
  }

  // 3. Update metadata, zero out memory, and add to page table
  page->page_id_ = page_id;
  page->pin_count_ = 1;
  page->is_dirty_ = false;
  page->ResetMemory();

  page_table_[page_id] = frame_id;

  // 4. Return pointer to the new page
  return page;
}

/**
 * Delete a page from the buffer pool.
 *
 * 0.  Make sure you call DeallocatePage!
 * 1.  Search the page table for the requested page (P).
 * 1.  If P does not exist, return true.
 * 2.  If P exists, but has a non-zero pin-count, return false. Someone is using the page.
 * 3.  Otherwise, P can be deleted. Remove P from the page table, reset its metadata
 *     and return it to the free list.
 */
bool BufferPoolManager::DeletePage(page_id_t page_id) {
  lock_guard<recursive_mutex> lock(latch_);

  // 0. De-allocate the page on disk
  DeallocatePage(page_id);

  // 1. Search the page table
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    // P does not exist in memory — nothing to delete from buffer pool
    return true;
  }

  frame_id_t frame_id = it->second;
  Page *page = &pages_[frame_id];

  // 2. If pin count is non-zero, someone is still using this page
  if (page->pin_count_ > 0) {
    return false;
  }

  // 3. Delete P: remove from page table, reset metadata, return to free list
  page_table_.erase(it);
  page->page_id_ = INVALID_PAGE_ID;
  page->pin_count_ = 0;
  page->is_dirty_ = false;
  page->ResetMemory();
  free_list_.push_back(frame_id);

  return true;
}

/**
 * Unpin a page in the buffer pool.
 * If the pin count reaches 0, the page becomes eligible for eviction
 * and is added to the replacer.
 *
 * @param page_id  The page to unpin.
 * @param is_dirty True if the page was modified while pinned.
 * @return true if the page was successfully unpinned, false otherwise.
 */
bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
  lock_guard<recursive_mutex> lock(latch_);

  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    return false;  // Page not in buffer pool
  }

  frame_id_t frame_id = it->second;
  Page *page = &pages_[frame_id];

  // Cannot unpin a page that is not pinned
  if (page->pin_count_ <= 0) {
    return false;
  }

  page->pin_count_--;

  // Update dirty flag if the page was modified
  if (is_dirty) {
    page->is_dirty_ = true;
  }

  // When pin count reaches zero, add to replacer (eligible for eviction)
  if (page->pin_count_ == 0) {
    replacer_->Unpin(frame_id);
  }

  return true;
}

/**
 * Flush a page to disk. The page stays in the buffer pool.
 *
 * @param page_id The page to flush.
 * @return true if the page was successfully flushed, false otherwise.
 */
bool BufferPoolManager::FlushPage(page_id_t page_id) {
  lock_guard<recursive_mutex> lock(latch_);

  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    return false;  // Page not in buffer pool
  }

  // Flush does not check pin count — write through even if pinned
  frame_id_t frame_id = it->second;
  Page *page = &pages_[frame_id];

  disk_manager_->WritePage(page_id, page->GetData());
  page->is_dirty_ = false;

  return true;
}

page_id_t BufferPoolManager::AllocatePage() {
  int next_page_id = disk_manager_->AllocatePage();
  return next_page_id;
}

void BufferPoolManager::DeallocatePage(__attribute__((unused)) page_id_t page_id) {
  disk_manager_->DeAllocatePage(page_id);
}

bool BufferPoolManager::IsPageFree(page_id_t page_id) {
  return disk_manager_->IsPageFree(page_id);
}

/**
 * Try to find a free page frame for replacement.
 * Always checks the free list first, then falls back to the replacer.
 *
 * @return A free frame id, or INVALID_FRAME_ID if no frame is available.
 */
frame_id_t BufferPoolManager::TryToFindFreePage() {
  // First, try to get a frame from the free list
  if (!free_list_.empty()) {
    frame_id_t frame_id = free_list_.front();
    free_list_.pop_front();
    return frame_id;
  }

  // Fall back to the replacer for a victim frame
  frame_id_t frame_id;
  if (replacer_->Victim(&frame_id)) {
    return frame_id;
  }

  // No free frame available — all pages are pinned
  return INVALID_FRAME_ID;
}

// Only used for debug
bool BufferPoolManager::CheckAllUnpinned() {
  bool res = true;
  for (size_t i = 0; i < pool_size_; i++) {
    if (pages_[i].pin_count_ != 0) {
      res = false;
      LOG(ERROR) << "page " << pages_[i].page_id_ << " pin count:" << pages_[i].pin_count_ << endl;
    }
  }
  return res;
}
