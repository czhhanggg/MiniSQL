#include "page/table_page.h"

// TODO: Update interface implementation if apply recovery

void TablePage::Init(page_id_t page_id, page_id_t prev_id, LogManager *log_mgr, Txn *txn) {
  memcpy(GetData(), &page_id, sizeof(page_id));
  SetPrevPageId(prev_id);
  SetNextPageId(INVALID_PAGE_ID);
  SetPrevInsertablePageId(INVALID_PAGE_ID);
  SetNextInsertablePageId(INVALID_PAGE_ID);
  SetFreeSpacePointer(PAGE_SIZE);
  SetTupleCount(0);
  SetLiveTupleCount(0);
  memset(GetData() + OFFSET_OCCUPIED_BITMAP, 0, BITMAP_BYTES);
  memset(GetData() + OFFSET_DELETED_BITMAP, 0, BITMAP_BYTES);
}

uint32_t TablePage::FindReusableSlot() {
  uint32_t tuple_count = GetTupleCount();
  ASSERT(tuple_count <= BITMAP_SLOT_CAPACITY, "Tuple slot count exceeds bitmap capacity.");
  for (uint32_t word_idx = 0; word_idx < BITMAP_WORD_COUNT; ++word_idx) {
    uint32_t base_slot = word_idx * BITMAP_WORD_BITS;
    if (base_slot >= tuple_count) {
      break;
    }
    uint64_t occupied_word = GetOccupiedBitmap()[word_idx];
    uint32_t valid_bits = tuple_count - base_slot;
    if (valid_bits < BITMAP_WORD_BITS) {
      uint64_t valid_mask = (1ULL << valid_bits) - 1;
      occupied_word |= ~valid_mask;
    }
    if (~occupied_word != 0) {
      for (uint32_t bit = 0; bit < BITMAP_WORD_BITS; ++bit) {
        uint32_t slot_num = base_slot + bit;
        if (slot_num >= tuple_count) {
          break;
        }
        if ((occupied_word & (1ULL << bit)) == 0) {
          return slot_num;
        }
      }
    }
  }
  return tuple_count;
}

int32_t TablePage::FindLiveSlot(uint32_t start_slot) {
  uint32_t tuple_count = GetTupleCount();
  ASSERT(tuple_count <= BITMAP_SLOT_CAPACITY, "Tuple slot count exceeds bitmap capacity.");
  if (start_slot >= tuple_count) {
    return -1;
  }

  uint32_t start_word = start_slot / BITMAP_WORD_BITS;
  for (uint32_t word_idx = start_word; word_idx < BITMAP_WORD_COUNT; ++word_idx) {
    uint32_t base_slot = word_idx * BITMAP_WORD_BITS;
    if (base_slot >= tuple_count) {
      break;
    }

    uint64_t live_word = GetOccupiedBitmap()[word_idx] & ~GetDeletedBitmap()[word_idx];
    uint32_t start_bit = word_idx == start_word ? start_slot % BITMAP_WORD_BITS : 0;
    if (start_bit != 0) {
      live_word &= (~0ULL << start_bit);
    }

    uint32_t valid_bits = tuple_count - base_slot;
    if (valid_bits < BITMAP_WORD_BITS) {
      uint64_t valid_mask = (1ULL << valid_bits) - 1;
      live_word &= valid_mask;
    }

    if (live_word != 0) {
      for (uint32_t bit = start_bit; bit < BITMAP_WORD_BITS; ++bit) {
        uint32_t slot_num = base_slot + bit;
        if (slot_num >= tuple_count) {
          break;
        }
        if ((live_word & (1ULL << bit)) != 0) {
          return static_cast<int32_t>(slot_num);
        }
      }
    }
  }
  return -1;
}

bool TablePage::CanHostAnyTuple() {
  bool has_reusable_slot = FindReusableSlot() < GetTupleCount();
  uint32_t min_required = has_reusable_slot ? 1U : static_cast<uint32_t>(SIZE_TUPLE + 1);
  return GetFreeSpaceRemaining() >= min_required;
}

bool TablePage::InsertTuple(Row &row, Schema *schema, Txn *txn, LockManager *lock_manager, LogManager *log_manager) {
  uint32_t serialized_size = row.GetSerializedSize(schema);
  ASSERT(serialized_size > 0, "Can not have empty row.");

  uint32_t slot_num = FindReusableSlot();
  bool reuse_existing_slot = slot_num < GetTupleCount();
  uint32_t required_space = serialized_size + (reuse_existing_slot ? 0U : static_cast<uint32_t>(SIZE_TUPLE));
  if (GetFreeSpaceRemaining() < required_space) {
    return false;
  }
  if (!reuse_existing_slot) {
    if (slot_num >= BITMAP_SLOT_CAPACITY) {
      return false;
    }
    SetTupleCount(slot_num + 1);
  }

  SetFreeSpacePointer(GetFreeSpacePointer() - serialized_size);
  uint32_t __attribute__((unused)) write_bytes = row.SerializeTo(GetData() + GetFreeSpacePointer(), schema);
  ASSERT(write_bytes == serialized_size, "Unexpected behavior in row serialize.");

  SetTupleOffsetAtSlot(slot_num, GetFreeSpacePointer());
  SetTupleSize(slot_num, serialized_size);
  SetSlotOccupied(slot_num, true);
  SetSlotDeleted(slot_num, false);
  SetLiveTupleCount(GetLiveTupleCount() + 1);
  row.SetRowId(RowId(GetTablePageId(), slot_num));
  return true;
}

bool TablePage::MarkDelete(const RowId &rid, Txn *txn, LockManager *lock_manager, LogManager *log_manager) {
  uint32_t slot_num = rid.GetSlotNum();
  if (slot_num >= GetTupleCount() || !IsSlotOccupied(slot_num)) {
    return false;
  }

  uint32_t tuple_size = GetTupleSize(slot_num);
  if (IsDeleted(tuple_size) || IsSlotDeleted(slot_num)) {
    return false;
  }

  SetTupleSize(slot_num, SetDeletedFlag(tuple_size));
  SetSlotDeleted(slot_num, true);
  SetLiveTupleCount(GetLiveTupleCount() - 1);
  return true;
}

bool TablePage::UpdateTuple(Row &new_row, Row *old_row, Schema *schema, Txn *txn, LockManager *lock_manager,
                            LogManager *log_manager) {
  ASSERT(old_row != nullptr && old_row->GetRowId().Get() != INVALID_ROWID.Get(), "invalid old row.");
  uint32_t serialized_size = new_row.GetSerializedSize(schema);
  ASSERT(serialized_size > 0, "Can not have empty row.");
  uint32_t slot_num = old_row->GetRowId().GetSlotNum();
  if (slot_num >= GetTupleCount() || !IsSlotOccupied(slot_num)) {
    return false;
  }

  uint32_t tuple_size = GetTupleSize(slot_num);
  if (IsDeleted(tuple_size) || IsSlotDeleted(slot_num)) {
    return false;
  }
  if (GetFreeSpaceRemaining() + tuple_size < serialized_size) {
    return false;
  }

  uint32_t tuple_offset = GetTupleOffsetAtSlot(slot_num);
  uint32_t __attribute__((unused)) read_bytes = old_row->DeserializeFrom(GetData() + tuple_offset, schema);
  ASSERT(tuple_size == read_bytes, "Unexpected behavior in tuple deserialize.");
  uint32_t free_space_pointer = GetFreeSpacePointer();
  ASSERT(tuple_offset >= free_space_pointer, "Offset should appear after current free space position.");
  memmove(GetData() + free_space_pointer + tuple_size - serialized_size, GetData() + free_space_pointer,
          tuple_offset - free_space_pointer);
  SetFreeSpacePointer(free_space_pointer + tuple_size - serialized_size);
  new_row.SerializeTo(GetData() + tuple_offset + tuple_size - serialized_size, schema);
  SetTupleSize(slot_num, serialized_size);

  for (uint32_t i = 0; i < GetTupleCount(); ++i) {
    uint32_t tuple_offset_i = GetTupleOffsetAtSlot(i);
    if (IsSlotOccupied(i) && tuple_offset_i < tuple_offset + tuple_size) {
      SetTupleOffsetAtSlot(i, tuple_offset_i + tuple_size - serialized_size);
    }
  }
  return true;
}

void TablePage::ApplyDelete(const RowId &rid, Txn *txn, LogManager *log_manager) {
  uint32_t slot_num = rid.GetSlotNum();
  ASSERT(slot_num < GetTupleCount(), "Cannot have more slots than tuples.");
  if (!IsSlotOccupied(slot_num)) {
    return;
  }

  uint32_t raw_tuple_size = GetTupleSize(slot_num);
  bool tuple_was_live = raw_tuple_size != 0 && !IsDeleted(raw_tuple_size);
  uint32_t tuple_offset = GetTupleOffsetAtSlot(slot_num);
  uint32_t tuple_size = raw_tuple_size;
  if (IsDeleted(tuple_size)) {
    tuple_size = UnsetDeletedFlag(tuple_size);
  }
  if (tuple_size == 0) {
    return;
  }

  uint32_t free_space_pointer = GetFreeSpacePointer();
  ASSERT(tuple_offset >= free_space_pointer, "Free space appears before tuples.");

  memmove(GetData() + free_space_pointer + tuple_size, GetData() + free_space_pointer,
          tuple_offset - free_space_pointer);
  SetFreeSpacePointer(free_space_pointer + tuple_size);
  SetTupleSize(slot_num, 0);
  SetTupleOffsetAtSlot(slot_num, 0);
  SetSlotOccupied(slot_num, false);
  SetSlotDeleted(slot_num, false);
  if (tuple_was_live) {
    SetLiveTupleCount(GetLiveTupleCount() - 1);
  }

  for (uint32_t i = 0; i < GetTupleCount(); ++i) {
    uint32_t tuple_offset_i = GetTupleOffsetAtSlot(i);
    if (IsSlotOccupied(i) && tuple_offset_i < tuple_offset) {
      SetTupleOffsetAtSlot(i, tuple_offset_i + tuple_size);
    }
  }
}

void TablePage::RollbackDelete(const RowId &rid, Txn *txn, LogManager *log_manager) {
  uint32_t slot_num = rid.GetSlotNum();
  ASSERT(slot_num < GetTupleCount(), "We can't have more slots than tuples.");
  if (!IsSlotOccupied(slot_num)) {
    return;
  }

  uint32_t tuple_size = GetTupleSize(slot_num);
  if (tuple_size != 0 && (IsDeleted(tuple_size) || IsSlotDeleted(slot_num))) {
    SetTupleSize(slot_num, UnsetDeletedFlag(tuple_size));
    if (IsSlotDeleted(slot_num)) {
      SetSlotDeleted(slot_num, false);
      SetLiveTupleCount(GetLiveTupleCount() + 1);
    }
  }
}

bool TablePage::GetTuple(Row *row, Schema *schema, Txn *txn, LockManager *lock_manager) {
  ASSERT(row != nullptr && row->GetRowId().Get() != INVALID_ROWID.Get(), "Invalid row.");
  uint32_t slot_num = row->GetRowId().GetSlotNum();
  if (slot_num >= GetTupleCount() || !IsSlotOccupied(slot_num)) {
    return false;
  }

  uint32_t tuple_size = GetTupleSize(slot_num);
  if (tuple_size == 0 || IsDeleted(tuple_size) || IsSlotDeleted(slot_num)) {
    return false;
  }

  uint32_t tuple_offset = GetTupleOffsetAtSlot(slot_num);
  uint32_t __attribute__((unused)) read_bytes = row->DeserializeFrom(GetData() + tuple_offset, schema);
  ASSERT(tuple_size == read_bytes, "Unexpected behavior in tuple deserialize.");
  return true;
}

bool TablePage::GetFirstTupleRid(RowId *first_rid) {
  if (GetLiveTupleCount() == 0) {
    first_rid->Set(INVALID_PAGE_ID, 0);
    return false;
  }

  int32_t slot_num = FindLiveSlot(0);
  if (slot_num < 0) {
    first_rid->Set(INVALID_PAGE_ID, 0);
    return false;
  }

  first_rid->Set(GetTablePageId(), static_cast<uint32_t>(slot_num));
  return true;
}

bool TablePage::GetNextTupleRid(const RowId &cur_rid, RowId *next_rid) {
  ASSERT(cur_rid.GetPageId() == GetTablePageId(), "Wrong table!");
  int32_t slot_num = FindLiveSlot(cur_rid.GetSlotNum() + 1);
  if (slot_num < 0) {
    next_rid->Set(INVALID_PAGE_ID, 0);
    return false;
  }

  next_rid->Set(GetTablePageId(), static_cast<uint32_t>(slot_num));
  return true;
}
