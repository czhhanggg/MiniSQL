#include "storage/table_heap.h"

void TableHeap::EnsureHeapMetadata() {
  if (heap_metadata_built_) {
    return;
  }

  insertable_head_page_id_ = INVALID_PAGE_ID;
  last_page_id_ = INVALID_PAGE_ID;
  page_id_t tail_insertable_page_id = INVALID_PAGE_ID;
  page_id_t page_id = first_page_id_;
  while (page_id != INVALID_PAGE_ID) {
    auto *page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));
    if (page == nullptr) {
      break;
    }

    page->WLatch();
    page_id_t next_page_id = page->GetNextPageId();
    last_page_id_ = page_id;
    page->SetPrevInsertablePageId(INVALID_PAGE_ID);
    page->SetNextInsertablePageId(INVALID_PAGE_ID);
    if (page->CanHostAnyTuple()) {
      if (tail_insertable_page_id != INVALID_PAGE_ID) {
        auto *tail_page =
            reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(tail_insertable_page_id));
        ASSERT(tail_page != nullptr, "Failed to fetch insertable tail page.");
        tail_page->WLatch();
        tail_page->SetNextInsertablePageId(page_id);
        tail_page->WUnlatch();
        buffer_pool_manager_->UnpinPage(tail_insertable_page_id, true);
        page->SetPrevInsertablePageId(tail_insertable_page_id);
      } else {
        insertable_head_page_id_ = page_id;
      }
      tail_insertable_page_id = page_id;
    }
    page->WUnlatch();
    buffer_pool_manager_->UnpinPage(page_id, true);
    page_id = next_page_id;
  }

  if (last_page_id_ == INVALID_PAGE_ID) {
    last_page_id_ = first_page_id_;
  }
  heap_metadata_built_ = true;
}

bool TableHeap::IsInsertablePageTracked(TablePage *page, page_id_t page_id) const {
  return page_id == insertable_head_page_id_ || page->GetPrevInsertablePageId() != INVALID_PAGE_ID ||
         page->GetNextInsertablePageId() != INVALID_PAGE_ID;
}

void TableHeap::LinkInsertablePage(TablePage *page, page_id_t page_id) {
  page_id_t old_head_page_id = insertable_head_page_id_;
  page->SetPrevInsertablePageId(INVALID_PAGE_ID);
  page->SetNextInsertablePageId(old_head_page_id);
  if (old_head_page_id != INVALID_PAGE_ID) {
    auto *head_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(old_head_page_id));
    ASSERT(head_page != nullptr, "Failed to fetch insertable head page.");
    head_page->WLatch();
    head_page->SetPrevInsertablePageId(page_id);
    head_page->WUnlatch();
    buffer_pool_manager_->UnpinPage(old_head_page_id, true);
  }
  insertable_head_page_id_ = page_id;
}

void TableHeap::UnlinkInsertablePage(TablePage *page, page_id_t page_id) {
  page_id_t prev_page_id = page->GetPrevInsertablePageId();
  page_id_t next_page_id = page->GetNextInsertablePageId();
  if (prev_page_id != INVALID_PAGE_ID) {
    auto *prev_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(prev_page_id));
    ASSERT(prev_page != nullptr, "Failed to fetch previous insertable page.");
    prev_page->WLatch();
    prev_page->SetNextInsertablePageId(next_page_id);
    prev_page->WUnlatch();
    buffer_pool_manager_->UnpinPage(prev_page_id, true);
  }
  if (next_page_id != INVALID_PAGE_ID) {
    auto *next_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(next_page_id));
    ASSERT(next_page != nullptr, "Failed to fetch next insertable page.");
    next_page->WLatch();
    next_page->SetPrevInsertablePageId(prev_page_id);
    next_page->WUnlatch();
    buffer_pool_manager_->UnpinPage(next_page_id, true);
  }
  if (insertable_head_page_id_ == page_id) {
    insertable_head_page_id_ = next_page_id;
  }
  page->SetPrevInsertablePageId(INVALID_PAGE_ID);
  page->SetNextInsertablePageId(INVALID_PAGE_ID);
}

void TableHeap::RefreshInsertablePage(TablePage *page, page_id_t page_id) {
  bool should_track = page->CanHostAnyTuple();
  bool is_tracked = IsInsertablePageTracked(page, page_id);
  if (should_track && !is_tracked) {
    LinkInsertablePage(page, page_id);
  } else if (!should_track && is_tracked) {
    UnlinkInsertablePage(page, page_id);
  }
}

bool TableHeap::InsertTuple(Row &row, Txn *txn) {
  if (row.GetSerializedSize(schema_) > TablePage::SIZE_MAX_ROW) {
    return false;
  }

  EnsureHeapMetadata();
  page_id_t page_id = insertable_head_page_id_;
  while (page_id != INVALID_PAGE_ID) {
    auto *page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));
    if (page == nullptr) {
      return false;
    }
    page->WLatch();
    page_id_t next_insertable_page_id = page->GetNextInsertablePageId();
    bool ok = page->InsertTuple(row, schema_, txn, lock_manager_, log_manager_);
    RefreshInsertablePage(page, page_id);
    page->WUnlatch();
    buffer_pool_manager_->UnpinPage(page_id, true);
    if (ok) {
      return true;
    }
    page_id = next_insertable_page_id;
  }

  page_id_t new_page_id = INVALID_PAGE_ID;
  auto *new_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(new_page_id));
  if (new_page == nullptr) {
    return false;
  }
  new_page->WLatch();
  new_page->Init(new_page_id, last_page_id_, log_manager_, txn);
  bool insert_ok = new_page->InsertTuple(row, schema_, txn, lock_manager_, log_manager_);
  if (insert_ok) {
    if (last_page_id_ != INVALID_PAGE_ID) {
      auto *last_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(last_page_id_));
      ASSERT(last_page != nullptr, "Failed to fetch last table page.");
      last_page->WLatch();
      last_page->SetNextPageId(new_page_id);
      last_page->WUnlatch();
      buffer_pool_manager_->UnpinPage(last_page_id_, true);
    }
    last_page_id_ = new_page_id;
    RefreshInsertablePage(new_page, new_page_id);
  }
  new_page->WUnlatch();
  buffer_pool_manager_->UnpinPage(new_page_id, true);
  return insert_ok;
}

bool TableHeap::MarkDelete(const RowId &rid, Txn *txn) {
  auto *page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  if (page == nullptr) {
    return false;
  }
  page->WLatch();
  bool ok = page->MarkDelete(rid, txn, lock_manager_, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), ok);
  return ok;
}

bool TableHeap::UpdateTuple(Row &row, const RowId &rid, Txn *txn) {
  EnsureHeapMetadata();
  auto *page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  if (page == nullptr) {
    return false;
  }
  Row old_row(rid);
  page->WLatch();
  bool ok = page->UpdateTuple(row, &old_row, schema_, txn, lock_manager_, log_manager_);
  if (ok) {
    RefreshInsertablePage(page, rid.GetPageId());
  }
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(rid.GetPageId(), ok);
  if (ok) {
    row.SetRowId(rid);
  }
  return ok;
}

void TableHeap::ApplyDelete(const RowId &rid, Txn *txn) {
  EnsureHeapMetadata();
  auto *page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  if (page == nullptr) {
    return;
  }
  page->WLatch();
  page->ApplyDelete(rid, txn, log_manager_);
  RefreshInsertablePage(page, rid.GetPageId());
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(rid.GetPageId(), true);
}

void TableHeap::RollbackDelete(const RowId &rid, Txn *txn) {
  auto *page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  assert(page != nullptr);
  page->WLatch();
  page->RollbackDelete(rid, txn, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
}

bool TableHeap::GetTuple(Row *row, Txn *txn) {
  auto *page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(row->GetRowId().GetPageId()));
  if (page == nullptr) {
    return false;
  }
  page->RLatch();
  bool ok = page->GetTuple(row, schema_, txn, lock_manager_);
  page->RUnlatch();
  buffer_pool_manager_->UnpinPage(row->GetRowId().GetPageId(), false);
  return ok;
}

void TableHeap::DeleteTable(page_id_t page_id) {
  if (page_id != INVALID_PAGE_ID) {
    auto temp_table_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));  // 删除table_heap
    if (temp_table_page->GetNextPageId() != INVALID_PAGE_ID)
      DeleteTable(temp_table_page->GetNextPageId());
    buffer_pool_manager_->UnpinPage(page_id, false);
    buffer_pool_manager_->DeletePage(page_id);
  } else {
    DeleteTable(first_page_id_);
  }
}

TableIterator TableHeap::Begin(Txn *txn) {
  page_id_t page_id = first_page_id_;
  while (page_id != INVALID_PAGE_ID) {
    auto *page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));
    if (page == nullptr) {
      return End();
    }
    RowId rid;
    bool found = page->GetFirstTupleRid(&rid);
    page_id_t next_page_id = page->GetNextPageId();
    buffer_pool_manager_->UnpinPage(page_id, false);
    if (found) {
      return TableIterator(this, rid, txn);
    }
    page_id = next_page_id;
  }
  return End();
}

TableIterator TableHeap::End() { return TableIterator(this, INVALID_ROWID, nullptr); }
