#include "buffer/lru_replacer.h"

#include "gtest/gtest.h"

TEST(LRUReplacerTest, SampleTest) {
  LRUReplacer lru_replacer(7);

  // Scenario: unpin six elements, i.e. add them to the replacer.
  lru_replacer.Unpin(1);
  lru_replacer.Unpin(2);
  lru_replacer.Unpin(3);
  lru_replacer.Unpin(4);
  lru_replacer.Unpin(5);
  lru_replacer.Unpin(6);
  lru_replacer.Unpin(1);
  EXPECT_EQ(6, lru_replacer.Size());

  // Scenario: get three victims from the lru.
  int value;
  lru_replacer.Victim(&value);
  EXPECT_EQ(1, value);
  lru_replacer.Victim(&value);
  EXPECT_EQ(2, value);
  lru_replacer.Victim(&value);
  EXPECT_EQ(3, value);

  // Scenario: pin elements in the replacer.
  // Note that 3 has already been victimized, so pinning 3 should have no effect.
  lru_replacer.Pin(3);
  lru_replacer.Pin(4);
  EXPECT_EQ(2, lru_replacer.Size());

  // Scenario: unpin 4. We expect that the reference bit of 4 will be set to 1.
  lru_replacer.Unpin(4);

  // Scenario: continue looking for victims. We expect these victims.
  lru_replacer.Victim(&value);
  EXPECT_EQ(5, value);
  lru_replacer.Victim(&value);
  EXPECT_EQ(6, value);
  lru_replacer.Victim(&value);
  EXPECT_EQ(4, value);
}

TEST(LRUReplacerTest, VictimOnEmptyTest) {
  LRUReplacer lru_replacer(3);

  int value;
  // Victim on empty replacer should return false
  EXPECT_FALSE(lru_replacer.Victim(&value));
  EXPECT_EQ(0, lru_replacer.Size());

  // Unpin one element then victim should succeed
  lru_replacer.Unpin(10);
  EXPECT_EQ(1, lru_replacer.Size());
  EXPECT_TRUE(lru_replacer.Victim(&value));
  EXPECT_EQ(0, lru_replacer.Size());

  // Now empty again
  EXPECT_FALSE(lru_replacer.Victim(&value));
}