#include "page/bitmap_page.h"

#include "glog/logging.h"

/**
 * Allocate a free page from the bitmap.
 * Scans from next_free_page_ hint, wrapping around if needed.
 * Sets the corresponding bit to 1 (allocated), increments page_allocated_,
 * updates next_free_page_ hint, and returns the page offset.
 *
 * @param page_offset[out] The index of the newly allocated page within the extent.
 * @return true if a page was successfully allocated, false if the extent is full.
 */
template <size_t PageSize>
bool BitmapPage<PageSize>::AllocatePage(uint32_t &page_offset) {
  // If all pages are already allocated, no free page available
  if (page_allocated_ >= GetMaxSupportedSize()) {
    return false;
  }

  // Scan from next_free_page_ hint, wrapping around
  for (uint32_t i = 0; i < GetMaxSupportedSize(); i++) {
    uint32_t idx = (next_free_page_ + i) % GetMaxSupportedSize();
    uint32_t byte_idx = idx / 8;
    uint8_t bit_idx = idx % 8;

    if (IsPageFreeLow(byte_idx, bit_idx)) {
      // Mark page as allocated (set bit to 1)
      bytes[byte_idx] |= (1 << bit_idx);
      page_allocated_++;
      // Move hint to next page for future allocations
      next_free_page_ = (idx + 1) % GetMaxSupportedSize();
      page_offset = idx;
      return true;
    }
  }

  return false;
}

/**
 * De-allocate a page in the bitmap.
 * Clears the corresponding bit to 0 (free), decrements page_allocated_,
 * and updates next_free_page_ hint.
 *
 * @param page_offset The index of the page to de-allocate within the extent.
 * @return true if the page was successfully de-allocated, false if it was already free.
 */
template <size_t PageSize>
bool BitmapPage<PageSize>::DeAllocatePage(uint32_t page_offset) {
  // Bounds check
  if (page_offset >= GetMaxSupportedSize()) {
    return false;
  }

  uint32_t byte_idx = page_offset / 8;
  uint8_t bit_idx = page_offset % 8;

  // If page is already free, cannot de-allocate again
  if (IsPageFreeLow(byte_idx, bit_idx)) {
    return false;
  }

  // Mark page as free (clear bit to 0)
  bytes[byte_idx] &= ~(1 << bit_idx);
  page_allocated_--;
  // Update hint so this page can be reused quickly
  next_free_page_ = page_offset;

  return true;
}

/**
 * Check whether a page is free (not allocated).
 *
 * @param page_offset The index of the page within the extent.
 * @return true if the page is free, false if it is allocated.
 */
template <size_t PageSize>
bool BitmapPage<PageSize>::IsPageFree(uint32_t page_offset) const {
  // Bounds check
  if (page_offset >= GetMaxSupportedSize()) {
    return false;
  }
  return IsPageFreeLow(page_offset / 8, page_offset % 8);
}

/**
 * Low-level helper: check whether a specific bit in the bitmap is 0 (free).
 *
 * @param byte_index The byte index within the bytes array (page_offset / 8).
 * @param bit_index  The bit index within that byte (page_offset % 8).
 * @return true if the bit is 0 (free), false if the bit is 1 (allocated).
 */
template <size_t PageSize>
bool BitmapPage<PageSize>::IsPageFreeLow(uint32_t byte_index, uint8_t bit_index) const {
  return (bytes[byte_index] & (1 << bit_index)) == 0;
}

template class BitmapPage<64>;

template class BitmapPage<128>;

template class BitmapPage<256>;

template class BitmapPage<512>;

template class BitmapPage<1024>;

template class BitmapPage<2048>;

template class BitmapPage<4096>;