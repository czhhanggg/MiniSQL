#include "storage/table_heap.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "record/field.h"
#include "record/schema.h"

namespace {

using Clock = std::chrono::steady_clock;

struct HeapContext {
  explicit HeapContext(const std::string &db_file) : db_file_name(db_file) {
    std::remove(db_file_name.c_str());
    disk_mgr = std::make_unique<DiskManager>(db_file_name);
    bpm = std::make_unique<BufferPoolManager>(DEFAULT_BUFFER_POOL_SIZE, disk_mgr.get());
    schema = BuildSchema();
    table_heap.reset(TableHeap::Create(bpm.get(), schema.get(), nullptr, nullptr, nullptr));
  }

  ~HeapContext() { std::remove(db_file_name.c_str()); }

  static std::shared_ptr<Schema> BuildSchema() {
    std::vector<Column *> columns = {new Column("id", TypeId::kTypeInt, 0, false, false),
                                     new Column("payload", TypeId::kTypeChar, 96, 1, true, false),
                                     new Column("score", TypeId::kTypeFloat, 2, true, false)};
    return std::make_shared<Schema>(columns);
  }

  Row BuildRow(int key) const {
    std::string payload(96, static_cast<char>('a' + (key % 26)));
    std::vector<Field> fields = {Field(TypeId::kTypeInt, key),
                                 Field(TypeId::kTypeChar, const_cast<char *>(payload.data()),
                                       static_cast<uint32_t>(payload.size()), true),
                                 Field(TypeId::kTypeFloat, static_cast<float>(key % 1000) / 7.0f)};
    return Row(fields);
  }

  std::string db_file_name;
  std::unique_ptr<DiskManager> disk_mgr;
  std::unique_ptr<BufferPoolManager> bpm;
  std::shared_ptr<Schema> schema;
  std::unique_ptr<TableHeap> table_heap;
};

page_id_t GetLastPageId(TableHeap *table_heap, BufferPoolManager *bpm) {
  page_id_t page_id = table_heap->GetFirstPageId();
  page_id_t last_page_id = page_id;
  while (page_id != INVALID_PAGE_ID) {
    auto *page = reinterpret_cast<TablePage *>(bpm->FetchPage(page_id));
    EXPECT_NE(page, nullptr);
    if (page == nullptr) {
      return INVALID_PAGE_ID;
    }
    last_page_id = page_id;
    page_id_t next_page_id = page->GetNextPageId();
    bpm->UnpinPage(last_page_id, false);
    page_id = next_page_id;
  }
  return last_page_id;
}

size_t CountTablePages(TableHeap *table_heap, BufferPoolManager *bpm) {
  size_t page_count = 0;
  page_id_t page_id = table_heap->GetFirstPageId();
  while (page_id != INVALID_PAGE_ID) {
    auto *page = reinterpret_cast<TablePage *>(bpm->FetchPage(page_id));
    EXPECT_NE(page, nullptr);
    if (page == nullptr) {
      return page_count;
    }
    page_id_t next_page_id = page->GetNextPageId();
    bpm->UnpinPage(page_id, false);
    page_id = next_page_id;
    page_count++;
  }
  return page_count;
}

void FillUntilPageCount(HeapContext *ctx, size_t target_page_count, int *next_key) {
  ASSERT_NE(ctx, nullptr);
  ASSERT_NE(next_key, nullptr);
  while (CountTablePages(ctx->table_heap.get(), ctx->bpm.get()) < target_page_count) {
    Row row = ctx->BuildRow((*next_key)++);
    ASSERT_TRUE(ctx->table_heap->InsertTuple(row, nullptr));
  }
}

bool LinearScanInsert(TableHeap *table_heap, BufferPoolManager *bpm, Schema *schema, Row &row,
                      size_t *visited_pages = nullptr) {
  if (row.GetSerializedSize(schema) > TablePage::SIZE_MAX_ROW) {
    return false;
  }

  page_id_t page_id = table_heap->GetFirstPageId();
  page_id_t last_page_id = INVALID_PAGE_ID;
  size_t local_visited_pages = 0;
  while (page_id != INVALID_PAGE_ID) {
    auto *page = reinterpret_cast<TablePage *>(bpm->FetchPage(page_id));
    if (page == nullptr) {
      return false;
    }
    local_visited_pages++;
    page->WLatch();
    page_id_t next_page_id = page->GetNextPageId();
    last_page_id = page_id;
    bool ok = page->InsertTuple(row, schema, nullptr, nullptr, nullptr);
    page->WUnlatch();
    bpm->UnpinPage(page_id, ok);
    if (ok) {
      if (visited_pages != nullptr) {
        *visited_pages = local_visited_pages;
      }
      return true;
    }
    page_id = next_page_id;
  }

  page_id_t new_page_id = INVALID_PAGE_ID;
  auto *new_page = reinterpret_cast<TablePage *>(bpm->NewPage(new_page_id));
  if (new_page == nullptr) {
    return false;
  }
  new_page->WLatch();
  new_page->Init(new_page_id, last_page_id, nullptr, nullptr);
  bool insert_ok = new_page->InsertTuple(row, schema, nullptr, nullptr, nullptr);
  if (insert_ok && last_page_id != INVALID_PAGE_ID) {
    auto *last_page = reinterpret_cast<TablePage *>(bpm->FetchPage(last_page_id));
    EXPECT_NE(last_page, nullptr);
    if (last_page != nullptr) {
      last_page->WLatch();
      last_page->SetNextPageId(new_page_id);
      last_page->WUnlatch();
      bpm->UnpinPage(last_page_id, true);
    }
  }
  new_page->WUnlatch();
  bpm->UnpinPage(new_page_id, true);
  if (visited_pages != nullptr) {
    *visited_pages = local_visited_pages;
  }
  return insert_ok;
}

}  // namespace

TEST(TableHeapPerfTest, InsertablePageListReducesInsertionSearchCost) {
  constexpr size_t kTargetPageCount = 128;
  constexpr int kBenchmarkInserts = 24;

  HeapContext baseline_ctx("table_heap_perf_baseline.db");
  HeapContext optimized_ctx("table_heap_perf_optimized.db");

  int baseline_key = 0;
  int optimized_key = 0;
  FillUntilPageCount(&baseline_ctx, kTargetPageCount, &baseline_key);
  FillUntilPageCount(&optimized_ctx, kTargetPageCount, &optimized_key);

  ASSERT_EQ(CountTablePages(baseline_ctx.table_heap.get(), baseline_ctx.bpm.get()), kTargetPageCount);
  ASSERT_EQ(CountTablePages(optimized_ctx.table_heap.get(), optimized_ctx.bpm.get()), kTargetPageCount);

  const page_id_t baseline_last_page_id = GetLastPageId(baseline_ctx.table_heap.get(), baseline_ctx.bpm.get());
  const page_id_t optimized_last_page_id = GetLastPageId(optimized_ctx.table_heap.get(), optimized_ctx.bpm.get());
  ASSERT_NE(baseline_last_page_id, INVALID_PAGE_ID);
  ASSERT_NE(optimized_last_page_id, INVALID_PAGE_ID);

  auto baseline_start = Clock::now();
  size_t total_linear_visited_pages = 0;
  for (int i = 0; i < kBenchmarkInserts; ++i) {
    Row row = baseline_ctx.BuildRow(baseline_key++);
    size_t visited_pages = 0;
    ASSERT_TRUE(LinearScanInsert(baseline_ctx.table_heap.get(), baseline_ctx.bpm.get(),
                                 baseline_ctx.schema.get(), row, &visited_pages));
    EXPECT_EQ(row.GetRowId().GetPageId(), baseline_last_page_id);
    EXPECT_EQ(visited_pages, kTargetPageCount);
    total_linear_visited_pages += visited_pages;
  }
  auto baseline_elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - baseline_start);

  auto optimized_start = Clock::now();
  for (int i = 0; i < kBenchmarkInserts; ++i) {
    Row row = optimized_ctx.BuildRow(optimized_key++);
    ASSERT_TRUE(optimized_ctx.table_heap->InsertTuple(row, nullptr));
    EXPECT_EQ(row.GetRowId().GetPageId(), optimized_last_page_id);
  }
  auto optimized_elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - optimized_start);

  std::cout << "[perf] total baseline visited pages: " << total_linear_visited_pages << std::endl;
  std::cout << "[perf] baseline linear insert time(us): " << baseline_elapsed.count() << std::endl;
  std::cout << "[perf] optimized insert time(us): " << optimized_elapsed.count() << std::endl;

  EXPECT_EQ(total_linear_visited_pages, kTargetPageCount * kBenchmarkInserts);
  EXPECT_LT(optimized_elapsed.count(), baseline_elapsed.count());
}
