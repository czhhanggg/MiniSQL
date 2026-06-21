#include "buffer/clock_replacer.h"

#include "gtest/gtest.h"

/**
 * SampleTest — basic pin/unpin/victim operations.
 *
 * This test follows the same scenario as LRUReplacerTest.SampleTest
 * to verify that CLOCKReplacer produces correct results. Although
 * the internal algorithms differ (Clock uses reference bits + hand
 * vs. LRU's ordered list), the eviction order should be consistent
 * for this sequence of operations.
 */
TEST(CLOCKReplacerTest, SampleTest) {
  CLOCKReplacer clock_replacer(7);

  // Scenario: unpin six elements, i.e. add them to the clock.
  clock_replacer.Unpin(1);
  clock_replacer.Unpin(2);
  clock_replacer.Unpin(3);
  clock_replacer.Unpin(4);
  clock_replacer.Unpin(5);
  clock_replacer.Unpin(6);
  clock_replacer.Unpin(1);  // 1 is already in the clock — refreshes its reference bit
  EXPECT_EQ(6, clock_replacer.Size());

  // Scenario: get three victims from the clock.
  // The hand makes a full rotation clearing all ref bits (1→0),
  // then picks the first frame with ref=0 (frame 1).
  int value;
  clock_replacer.Victim(&value);
  EXPECT_EQ(1, value);
  clock_replacer.Victim(&value);
  EXPECT_EQ(2, value);
  clock_replacer.Victim(&value);
  EXPECT_EQ(3, value);

  // Scenario: pin elements in the replacer.
  // Note that 3 has already been victimized, so pinning 3 has no effect.
  clock_replacer.Pin(3);
  clock_replacer.Pin(4);  // Removes 4 from the clock
  EXPECT_EQ(2, clock_replacer.Size());

  // Scenario: unpin 4. We expect that the reference bit of 4 will be set to 1.
  clock_replacer.Unpin(4);

  // Scenario: continue looking for victims. We expect these victims.
  clock_replacer.Victim(&value);
  EXPECT_EQ(5, value);
  clock_replacer.Victim(&value);
  EXPECT_EQ(6, value);
  clock_replacer.Victim(&value);
  EXPECT_EQ(4, value);
}

/**
 * VictimOnEmptyTest — verify that Victim on an empty clock returns false.
 */
TEST(CLOCKReplacerTest, VictimOnEmptyTest) {
  CLOCKReplacer clock_replacer(3);

  int value;
  // Victim on empty replacer should return false
  EXPECT_FALSE(clock_replacer.Victim(&value));
  EXPECT_EQ(0, clock_replacer.Size());

  // Unpin one element then victim should succeed
  clock_replacer.Unpin(2);
  EXPECT_EQ(1, clock_replacer.Size());
  EXPECT_TRUE(clock_replacer.Victim(&value));
  EXPECT_EQ(0, clock_replacer.Size());

  // Now empty again
  EXPECT_FALSE(clock_replacer.Victim(&value));
}

/**
 * SecondChanceTest — demonstrates the Clock algorithm's defining feature:
 * the reference bit gives frequently accessed pages a "second chance"
 * before eviction.
 *
 * Scenario:
 *   1. Add frames 0, 1, 2, 3 to the clock (all with ref=1).
 *   2. Victimize 1, 2, 3 — only frame 0 remains with ref=0.
 *   3. Add frame 4 with ref=1.
 *   4. Re-unpin frame 0 — this sets its reference bit back to 1
 *      (the "second chance").
 *   5. Now both 0 and 4 have ref=1. The hand will clear both bits
 *      on the first pass, then evict 4 on the second pass.
 *   6. Frame 0, having received a second chance, survives longer.
 */
TEST(CLOCKReplacerTest, SecondChanceTest) {
  CLOCKReplacer clock_replacer(7);

  // Step 1: Add frames 0-3 to the clock
  clock_replacer.Unpin(0);
  clock_replacer.Unpin(1);
  clock_replacer.Unpin(2);
  clock_replacer.Unpin(3);
  EXPECT_EQ(4, clock_replacer.Size());

  // Step 2: Victimize three frames
  int value;
  clock_replacer.Victim(&value);
  EXPECT_EQ(1, value);
  clock_replacer.Victim(&value);
  EXPECT_EQ(2, value);
  clock_replacer.Victim(&value);
  EXPECT_EQ(3, value);
  EXPECT_EQ(1, clock_replacer.Size());  // Only frame 0 remains

  // Step 3: Add frame 4 with ref=1
  clock_replacer.Unpin(4);
  EXPECT_EQ(2, clock_replacer.Size());  // Frames 0 and 4

  // Step 4: Give frame 0 a second chance by re-unpinning it
  // This sets ref[0] back to 1.
  clock_replacer.Unpin(0);

  // Step 5: Victim.
  // The clock hand (at position 3 after victimizing 3) advances:
  //   hand=4: in_clock[4]=T, ref[4]=1 → clear to 0
  //   hand=5,6: skip (not in clock)
  //   hand=0: in_clock[0]=T, ref[0]=1 → clear to 0
  //   hand=1,2,3: skip (already victimized)
  //   hand=4: in_clock[4]=T, ref[4]=0 → VICTIM 4
  // Frame 0 survives this round because of its second chance!
  clock_replacer.Victim(&value);
  EXPECT_EQ(4, value);
  EXPECT_EQ(1, clock_replacer.Size());  // Only frame 0 remains

  // Step 6: The last remaining frame is 0.
  clock_replacer.Victim(&value);
  EXPECT_EQ(0, value);
  EXPECT_EQ(0, clock_replacer.Size());

  // Verify empty
  EXPECT_FALSE(clock_replacer.Victim(&value));
}

/**
 * PinRemovesFromClockTest — verify that pinning a frame correctly
 * removes it from the eviction pool and that it can be re-added
 * by unpinning.
 */
TEST(CLOCKReplacerTest, PinRemovesFromClockTest) {
  CLOCKReplacer clock_replacer(5);

  // Add frames to clock
  clock_replacer.Unpin(0);
  clock_replacer.Unpin(1);
  clock_replacer.Unpin(2);
  EXPECT_EQ(3, clock_replacer.Size());

  // Pin frame 1 — should remove it
  clock_replacer.Pin(1);
  EXPECT_EQ(2, clock_replacer.Size());

  // Pin non-existent frame — no-op
  clock_replacer.Pin(99);  // Out of range, should be ignored
  EXPECT_EQ(2, clock_replacer.Size());

  // Pin already-removed frame — no-op
  clock_replacer.Pin(1);
  EXPECT_EQ(2, clock_replacer.Size());

  // Unpin frame 1 — should re-add with ref=1
  clock_replacer.Unpin(1);
  EXPECT_EQ(3, clock_replacer.Size());

  // Verify all frames are evictable
  int count = 0;
  int value;
  while (clock_replacer.Victim(&value)) {
    count++;
  }
  EXPECT_EQ(3, count);
  EXPECT_EQ(0, clock_replacer.Size());
}

/**
 * ConcurrentPinUnpinTest — verify that interleaved pin and unpin
 * operations maintain correct size tracking.
 */
TEST(CLOCKReplacerTest, ConcurrentPinUnpinTest) {
  CLOCKReplacer clock_replacer(10);

  // Add all frames
  for (int i = 0; i < 10; i++) {
    clock_replacer.Unpin(i);
  }
  EXPECT_EQ(10, clock_replacer.Size());

  // Pin every other frame
  for (int i = 0; i < 10; i += 2) {
    clock_replacer.Pin(i);
  }
  EXPECT_EQ(5, clock_replacer.Size());

  // Unpin them back
  for (int i = 0; i < 10; i += 2) {
    clock_replacer.Unpin(i);
  }
  EXPECT_EQ(10, clock_replacer.Size());

  // All frames should be evictable
  int count = 0;
  int value;
  while (clock_replacer.Victim(&value)) {
    count++;
  }
  EXPECT_EQ(10, count);
}

/**
 * VictimFullRotationTest — verify that the clock hand wraps around
 * correctly and eventually finds a victim even when all frames
 * initially have their reference bits set.
 */
TEST(CLOCKReplacerTest, VictimFullRotationTest) {
  CLOCKReplacer clock_replacer(100);

  // Add all 100 frames
  for (int i = 0; i < 100; i++) {
    clock_replacer.Unpin(i);
  }
  EXPECT_EQ(100, clock_replacer.Size());

  // Victimize all frames — each call should succeed
  for (int i = 0; i < 100; i++) {
    int value;
    EXPECT_TRUE(clock_replacer.Victim(&value));
    // Verify the victim is a valid frame id
    EXPECT_GE(value, 0);
    EXPECT_LT(value, 100);
  }

  // Should be empty now
  EXPECT_EQ(0, clock_replacer.Size());
  int value;
  EXPECT_FALSE(clock_replacer.Victim(&value));
}
