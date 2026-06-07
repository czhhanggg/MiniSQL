#include "storage/disk_manager.h"

#include <sys/stat.h>

#include <filesystem>
#include <stdexcept>

#include "glog/logging.h"
#include "page/bitmap_page.h"

DiskManager::DiskManager(const std::string &db_file) : file_name_(db_file) {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);
  db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
  // directory or file does not exist
  if (!db_io_.is_open()) {
    db_io_.clear();
    // create a new file
    std::filesystem::path p = db_file;
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
    db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
    db_io_.close();
    // reopen with original mode
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
    if (!db_io_.is_open()) {
      throw std::exception();
    }
  }
  ReadPhysicalPage(META_PAGE_ID, meta_data_);
}

void DiskManager::Close() {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);
  WritePhysicalPage(META_PAGE_ID, meta_data_);
  if (!closed) {
    db_io_.close();
    closed = true;
  }
}

void DiskManager::ReadPage(page_id_t logical_page_id, char *page_data) {
  ASSERT(logical_page_id >= 0, "Invalid page id.");
  ReadPhysicalPage(MapPageId(logical_page_id), page_data);
}

void DiskManager::WritePage(page_id_t logical_page_id, const char *page_data) {
  ASSERT(logical_page_id >= 0, "Invalid page id.");
  WritePhysicalPage(MapPageId(logical_page_id), page_data);
}

/**
 * Map a logical page ID to its physical page ID on disk.
 *
 * Disk layout (each extent holds BITMAP_SIZE data pages):
 *   | Meta(0) | Bitmap0(1) | Data0..Data_{N-1}(2..N+1) |
 *   | Bitmap1(N+2) | Data_N..Data_{2N+1}(N+3..2N+2) | ...
 *
 * For logical page L:
 *   extent_id = L / BITMAP_SIZE
 *   offset    = L % BITMAP_SIZE
 *   physical  = 1 (meta) + extent_id * (1 bitmap + BITMAP_SIZE data) + 1 (bitmap) + offset
 *             = 2 + extent_id * (BITMAP_SIZE + 1) + offset
 */
page_id_t DiskManager::MapPageId(page_id_t logical_page_id) {
  uint32_t extent_id = static_cast<uint32_t>(logical_page_id) / BITMAP_SIZE;
  uint32_t page_offset = static_cast<uint32_t>(logical_page_id) % BITMAP_SIZE;
  return 2 + extent_id * (BITMAP_SIZE + 1) + page_offset;
}

/**
 * Allocate a new page on disk.
 * Scans existing extents for a free page; if all are full, creates a new extent.
 *
 * @return The logical page ID of the newly allocated page, or INVALID_PAGE_ID on failure.
 */
page_id_t DiskManager::AllocatePage() {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);

  DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(meta_data_);

  // 1. Scan existing extents for a free page
  for (uint32_t extent_id = 0; extent_id < meta_page->GetExtentNums(); extent_id++) {
    // Skip full extents
    if (meta_page->GetExtentUsedPage(extent_id) == BITMAP_SIZE) {
      continue;
    }

    // Read the bitmap page for this extent
    page_id_t bitmap_physical_id = 1 + extent_id * (BITMAP_SIZE + 1);
    char bitmap_buf[PAGE_SIZE];
    ReadPhysicalPage(bitmap_physical_id, bitmap_buf);
    auto *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_buf);

    uint32_t page_offset;
    if (bitmap->AllocatePage(page_offset)) {
      // Write updated bitmap back to disk
      WritePhysicalPage(bitmap_physical_id, bitmap_buf);

      // Update meta page
      meta_page->num_allocated_pages_++;
      meta_page->extent_used_page_[extent_id]++;

      return static_cast<page_id_t>(extent_id * BITMAP_SIZE + page_offset);
    }
  }

  // 2. No free page found — create a new extent
  uint32_t extent_id = meta_page->GetExtentNums();

  // Initialize a fresh bitmap page (all zeros = all pages free)
  char bitmap_buf[PAGE_SIZE];
  memset(bitmap_buf, 0, PAGE_SIZE);
  auto *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_buf);

  uint32_t page_offset;
  bool success = bitmap->AllocatePage(page_offset);
  ASSERT(success, "A freshly created bitmap must have at least one free page.");

  // Write the new bitmap page to disk
  page_id_t bitmap_physical_id = 1 + extent_id * (BITMAP_SIZE + 1);
  WritePhysicalPage(bitmap_physical_id, bitmap_buf);

  // Update meta page
  meta_page->num_allocated_pages_++;
  meta_page->num_extents_++;
  meta_page->extent_used_page_[extent_id] = 1;

  return static_cast<page_id_t>(extent_id * BITMAP_SIZE + page_offset);
}

/**
 * De-allocate a logical page on disk.
 * Clears the corresponding bit in the extent's bitmap and updates the meta page.
 */
void DiskManager::DeAllocatePage(page_id_t logical_page_id) {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);

  DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(meta_data_);

  uint32_t extent_id = static_cast<uint32_t>(logical_page_id) / BITMAP_SIZE;
  uint32_t page_offset = static_cast<uint32_t>(logical_page_id) % BITMAP_SIZE;

  // If the extent doesn't exist, nothing to de-allocate
  if (extent_id >= meta_page->GetExtentNums()) {
    LOG(WARNING) << "Attempt to de-allocate page " << logical_page_id
                 << " but extent " << extent_id << " does not exist." << std::endl;
    return;
  }

  // Read the bitmap page for this extent
  page_id_t bitmap_physical_id = 1 + extent_id * (BITMAP_SIZE + 1);
  char bitmap_buf[PAGE_SIZE];
  ReadPhysicalPage(bitmap_physical_id, bitmap_buf);
  auto *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_buf);

  // De-allocate the page in the bitmap
  if (bitmap->DeAllocatePage(page_offset)) {
    // Write updated bitmap back to disk
    WritePhysicalPage(bitmap_physical_id, bitmap_buf);

    // Update meta page
    meta_page->num_allocated_pages_--;
    meta_page->extent_used_page_[extent_id]--;
  }
}

/**
 * Check whether a logical page is free on disk.
 *
 * @return true if the page is free (or the extent doesn't exist yet), false otherwise.
 */
bool DiskManager::IsPageFree(page_id_t logical_page_id) {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);

  DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(meta_data_);

  uint32_t extent_id = static_cast<uint32_t>(logical_page_id) / BITMAP_SIZE;
  uint32_t page_offset = static_cast<uint32_t>(logical_page_id) % BITMAP_SIZE;

  // If the extent doesn't exist yet, the page is definitely free
  if (extent_id >= meta_page->GetExtentNums()) {
    return true;
  }

  // Read the bitmap and check the specific page
  page_id_t bitmap_physical_id = 1 + extent_id * (BITMAP_SIZE + 1);
  char bitmap_buf[PAGE_SIZE];
  ReadPhysicalPage(bitmap_physical_id, bitmap_buf);
  auto *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(bitmap_buf);

  return bitmap->IsPageFree(page_offset);
}

int DiskManager::GetFileSize(const std::string &file_name) {
  struct stat stat_buf;
  int rc = stat(file_name.c_str(), &stat_buf);
  return rc == 0 ? stat_buf.st_size : -1;
}

void DiskManager::ReadPhysicalPage(page_id_t physical_page_id, char *page_data) {
  int offset = physical_page_id * PAGE_SIZE;
  // check if read beyond file length
  if (offset >= GetFileSize(file_name_)) {
#ifdef ENABLE_BPM_DEBUG
    LOG(INFO) << "Read less than a page" << std::endl;
#endif
    memset(page_data, 0, PAGE_SIZE);
  } else {
    // set read cursor to offset
    db_io_.seekp(offset);
    db_io_.read(page_data, PAGE_SIZE);
    // if file ends before reading PAGE_SIZE
    int read_count = db_io_.gcount();
    if (read_count < PAGE_SIZE) {
#ifdef ENABLE_BPM_DEBUG
      LOG(INFO) << "Read less than a page" << std::endl;
#endif
      memset(page_data + read_count, 0, PAGE_SIZE - read_count);
    }
  }
}

void DiskManager::WritePhysicalPage(page_id_t physical_page_id, const char *page_data) {
  size_t offset = static_cast<size_t>(physical_page_id) * PAGE_SIZE;
  // set write cursor to offset
  db_io_.seekp(offset);
  db_io_.write(page_data, PAGE_SIZE);
  // check for I/O error
  if (db_io_.bad()) {
    LOG(ERROR) << "I/O error while writing";
    return;
  }
  // needs to flush to keep disk file in sync
  db_io_.flush();
}