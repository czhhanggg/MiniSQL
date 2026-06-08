#include "storage/table_iterator.h"

#include "common/macros.h"
#include "storage/table_heap.h"

TableIterator::TableIterator(TableHeap *table_heap, RowId rid, Txn *txn)
    : table_heap_(table_heap), row_(rid), txn_(txn), is_end_(rid.Get() == INVALID_ROWID.Get()) {
  if (!is_end_ && table_heap_ != nullptr) {
    if (!table_heap_->GetTuple(&row_, txn_)) {
      is_end_ = true;
      row_.SetRowId(INVALID_ROWID);
    }
  }
}

TableIterator::TableIterator(const TableIterator &other) {
  table_heap_ = other.table_heap_;
  row_ = other.row_;
  txn_ = other.txn_;
  is_end_ = other.is_end_;
}

TableIterator::~TableIterator() {
}

bool TableIterator::operator==(const TableIterator &itr) const {
  if (is_end_ && itr.is_end_) {
    return true;
  }
  return table_heap_ == itr.table_heap_ && row_.GetRowId().Get() == itr.row_.GetRowId().Get() && is_end_ == itr.is_end_;
}

bool TableIterator::operator!=(const TableIterator &itr) const {
  return !(*this == itr);
}

const Row &TableIterator::operator*() {
  return row_;
}

Row *TableIterator::operator->() {
  return &row_;
}

TableIterator &TableIterator::operator=(const TableIterator &itr) noexcept {
  if (this != &itr) {
    table_heap_ = itr.table_heap_;
    row_ = itr.row_;
    txn_ = itr.txn_;
    is_end_ = itr.is_end_;
  }
  return *this;
}

// ++iter
TableIterator &TableIterator::operator++() {
  if (is_end_ || table_heap_ == nullptr) {
    return *this;
  }

  auto current_rid = row_.GetRowId();
  auto *page = reinterpret_cast<TablePage *>(table_heap_->buffer_pool_manager_->FetchPage(current_rid.GetPageId()));
  if (page == nullptr) {
    is_end_ = true;
    row_.SetRowId(INVALID_ROWID);
    return *this;
  }

  RowId next_rid;
  bool found = page->GetNextTupleRid(current_rid, &next_rid);
  page_id_t next_page_id = page->GetNextPageId();
  table_heap_->buffer_pool_manager_->UnpinPage(current_rid.GetPageId(), false);

  if (!found) {
    while (next_page_id != INVALID_PAGE_ID) {
      auto *next_page =
          reinterpret_cast<TablePage *>(table_heap_->buffer_pool_manager_->FetchPage(next_page_id));
      if (next_page == nullptr) {
        break;
      }
      found = next_page->GetFirstTupleRid(&next_rid);
      page_id_t temp_next = next_page->GetNextPageId();
      table_heap_->buffer_pool_manager_->UnpinPage(next_page_id, false);
      if (found) {
        break;
      }
      next_page_id = temp_next;
    }
  }

  if (!found) {
    is_end_ = true;
    row_.SetRowId(INVALID_ROWID);
    row_.destroy();
    return *this;
  }

  row_.destroy();
  row_.SetRowId(next_rid);
  is_end_ = !table_heap_->GetTuple(&row_, txn_);
  return *this;
}

// iter++
TableIterator TableIterator::operator++(int) {
  TableIterator temp(*this);
  ++(*this);
  return temp;
}
