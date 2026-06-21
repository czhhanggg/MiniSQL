#include "buffer/clock_replacer.h"

/**
 * Create a new CLOCKReplacer.
 * @param num_pages the maximum number of frames this replacer will track
 */
CLOCKReplacer::CLOCKReplacer(size_t num_pages)
    : capacity_(num_pages),
      ref_bits_(num_pages, false),
      in_clock_(num_pages, false),
      clock_hand_(0),
      size_(0) {}

CLOCKReplacer::~CLOCKReplacer() = default;

/**
 * Remove the victim frame according to the Clock replacement policy.
 *
 * The clock hand scans forward through evictable frames:
 * - If the reference bit is 1, clear it to 0 (give a "second chance")
 *   and advance the hand.
 * - If the reference bit is 0, this frame is the victim — remove it
 *   from the clock and return its frame_id.
 *
 * @param[out] frame_id The frame id of the victim.
 * @return true if a victim was found, false if the clock is empty.
 */
bool CLOCKReplacer::Victim(frame_id_t *frame_id) {
  lock_guard<mutex> lock(latch_);

  if (size_ == 0) {
    return false;
  }

  // Scan for a victim. The loop is guaranteed to terminate because:
  // - Each frame with ref bit = 1 gets it cleared to 0 (at most once per
  //   full rotation), which reduces the number of "second chance" frames.
  // - After clearing all ref bits, the next evictable frame with ref bit = 0
  //   is chosen as the victim.
  //
  // In the worst case the hand makes up to 2 full rotations before finding
  // a victim, which is O(capacity).
  while (true) {
    // Advance the clock hand (wrap around)
    clock_hand_ = (clock_hand_ + 1) % capacity_;

    if (in_clock_[clock_hand_]) {
      if (ref_bits_[clock_hand_]) {
        // Give this frame a second chance — clear the reference bit
        ref_bits_[clock_hand_] = false;
      } else {
        // Reference bit is 0 — this frame is the victim
        in_clock_[clock_hand_] = false;
        ref_bits_[clock_hand_] = false;
        size_--;
        *frame_id = static_cast<frame_id_t>(clock_hand_);
        return true;
      }
    }
    // else: frame is not in the clock, skip it
  }
}

/**
 * Pin a frame, removing it from the eviction pool.
 * If the frame is not currently in the clock, this is a no-op.
 *
 * @param frame_id The frame id to pin.
 */
void CLOCKReplacer::Pin(frame_id_t frame_id) {
  lock_guard<mutex> lock(latch_);

  // Validate frame_id
  if (static_cast<size_t>(frame_id) >= capacity_) {
    return;
  }

  if (in_clock_[frame_id]) {
    // Remove from the clock — frame is now pinned and not evictable
    in_clock_[frame_id] = false;
    ref_bits_[frame_id] = false;
    size_--;
  }
  // else: frame is not in the clock, no-op
}

/**
 * Unpin a frame, making it eligible for eviction.
 *
 * If the frame is not currently in the clock, it is added with its
 * reference bit set to 1 (giving it a "second chance" before eviction).
 * If the frame is already in the clock, this is a no-op (it keeps its
 * current reference bit and position).
 *
 * @param frame_id The frame id to unpin.
 */
void CLOCKReplacer::Unpin(frame_id_t frame_id) {
  lock_guard<mutex> lock(latch_);

  // Validate frame_id
  if (static_cast<size_t>(frame_id) >= capacity_) {
    return;
  }

  if (!in_clock_[frame_id]) {
    // Add to the clock with reference bit set to 1
    in_clock_[frame_id] = true;
    ref_bits_[frame_id] = true;
    size_++;
  } else {
    // Frame is already in the clock — refresh its reference bit.
    // This is the "second chance" mechanism: recently accessed frames
    // survive one full rotation of the clock hand before eviction.
    ref_bits_[frame_id] = true;
  }
}

/**
 * Return the number of evictable frames currently tracked by the clock.
 */
size_t CLOCKReplacer::Size() {
  lock_guard<mutex> lock(latch_);
  return size_;
}
