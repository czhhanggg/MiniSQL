#include "buffer/lru_replacer.h"

LRUReplacer::LRUReplacer(size_t num_pages) {}

LRUReplacer::~LRUReplacer() = default;

/**
 * Remove the victim frame (the least recently used) from the replacer.
 * The front of lru_list_ is the LRU element.
 *
 * @param[out] frame_id The frame id of the removed victim.
 * @return true if a victim was found, false if the replacer is empty.
 */
bool LRUReplacer::Victim(frame_id_t *frame_id) {
  lock_guard<mutex> lock(latch_);

  if (lru_list_.empty()) {
    return false;
  }

  // Front of the list is the least recently used
  *frame_id = lru_list_.front();
  lru_map_.erase(*frame_id);
  lru_list_.pop_front();

  return true;
}

/**
 * Pin a frame, removing it from the replacer so it cannot be victimized.
 * If the frame is not in the replacer, this is a no-op.
 *
 * @param frame_id The frame id to pin.
 */
void LRUReplacer::Pin(frame_id_t frame_id) {
  lock_guard<mutex> lock(latch_);

  auto it = lru_map_.find(frame_id);
  if (it != lru_map_.end()) {
    lru_list_.erase(it->second);
    lru_map_.erase(it);
  }
}

/**
 * Unpin a frame, making it eligible for eviction.
 * If the frame is already in the replacer, this is a NO-OP (it stays at
 * its current position and is NOT moved to MRU).
 * Otherwise, the frame is added to the back (MRU position).
 *
 * @param frame_id The frame id to unpin.
 */
void LRUReplacer::Unpin(frame_id_t frame_id) {
  lock_guard<mutex> lock(latch_);

  // If already in replacer, NO-OP (do not move to MRU)
  if (lru_map_.find(frame_id) != lru_map_.end()) {
    return;
  }

  // Add to back (MRU position)
  lru_list_.push_back(frame_id);
  lru_map_[frame_id] = prev(lru_list_.end());
}

/**
 * Return the current number of frames in the replacer that can be victimized.
 */
size_t LRUReplacer::Size() {
  lock_guard<mutex> lock(latch_);
  return lru_list_.size();
}