#ifndef MINISQL_CLOCK_REPLACER_H
#define MINISQL_CLOCK_REPLACER_H

#include <mutex>
#include <vector>

#include "buffer/replacer.h"
#include "common/config.h"

using namespace std;

/**
 * CLOCKReplacer implements the Clock replacement policy.
 *
 * The Clock algorithm is an approximation of LRU that uses a circular
 * buffer with reference bits and a clock hand:
 *
 * - Each evictable frame has a reference bit (initially set to 1).
 * - When a victim is needed, the clock hand scans forward:
 *     * If the reference bit is 1, it is cleared to 0 and the hand advances
 *       (the frame gets a "second chance").
 *     * If the reference bit is 0, that frame is chosen as the victim.
 * - Pin() removes a frame from the clock entirely (no longer evictable).
 * - Unpin() adds a frame to the clock with its reference bit set to 1.
 *
 * This provides O(1) Pin/Unpin and O(1) amortized Victim operations.
 */
class CLOCKReplacer : public Replacer {
 public:
  /**
   * Create a new CLOCKReplacer.
   * @param num_pages the maximum number of pages the CLOCKReplacer will track
   */
  explicit CLOCKReplacer(size_t num_pages);

  /**
   * Destroys the CLOCKReplacer.
   */
  ~CLOCKReplacer() override;

  /**
   * Remove the victim frame as defined by the Clock replacement policy.
   * @param[out] frame_id id of the frame that was removed
   * @return true if a victim frame was found, false otherwise
   */
  bool Victim(frame_id_t *frame_id) override;

  /**
   * Pins a frame, removing it from the eviction pool.
   * If the frame is not in the clock, this is a no-op.
   * @param frame_id the id of the frame to pin
   */
  void Pin(frame_id_t frame_id) override;

  /**
   * Unpins a frame, making it eligible for eviction.
   * The frame is added to the clock with its reference bit set to 1.
   * If the frame is already in the clock, its reference bit is set to 1
   * (giving it a "second chance").
   * @param frame_id the id of the frame to unpin
   */
  void Unpin(frame_id_t frame_id) override;

  /** @return the number of evictable frames currently in the clock */
  size_t Size() override;

 private:
  size_t capacity_;           // maximum number of frames
  vector<bool> ref_bits_;     // reference bits, indexed by frame_id
  vector<bool> in_clock_;     // whether each frame is in the clock (evictable)
  size_t clock_hand_;         // current position of the clock hand
  size_t size_;               // number of frames currently evictable
  mutex latch_;               // protects shared data structures
};

#endif  // MINISQL_CLOCK_REPLACER_H
