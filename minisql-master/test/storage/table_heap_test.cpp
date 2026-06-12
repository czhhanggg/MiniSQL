#include "storage/table_heap.h"

#include <unordered_map>
#include <vector>

#include "common/instance.h"
#include "gtest/gtest.h"
#include "record/field.h"
#include "record/schema.h"
#include "utils/utils.h"

static string db_file_name = "table_heap_test.db";
using Fields = std::vector<Field>;

TEST(TableHeapTest, TableHeapSampleTest) {
  // init testing instance
  remove(db_file_name.c_str());
  auto disk_mgr_ = new DiskManager(db_file_name);
  auto bpm_ = new BufferPoolManager(DEFAULT_BUFFER_POOL_SIZE, disk_mgr_);
  const int row_nums = 10000;
  // create schema
  std::vector<Column *> columns = {new Column("id", TypeId::kTypeInt, 0, false, false),
                                   new Column("name", TypeId::kTypeChar, 64, 1, true, false),
                                   new Column("account", TypeId::kTypeFloat, 2, true, false)};
  auto schema = std::make_shared<Schema>(columns);
  // create rows
  std::unordered_map<int64_t, Fields *> row_values;
  uint32_t size = 0;
  TableHeap *table_heap = TableHeap::Create(bpm_, schema.get(), nullptr, nullptr, nullptr);
  for (int i = 0; i < row_nums; i++) {
    int32_t len = RandomUtils::RandomInt(0, 64);
    char *characters = new char[len];
    RandomUtils::RandomString(characters, len);
    Fields *fields =
        new Fields{Field(TypeId::kTypeInt, i), Field(TypeId::kTypeChar, const_cast<char *>(characters), len, true),
                   Field(TypeId::kTypeFloat, RandomUtils::RandomFloat(-999.f, 999.f))};
    Row row(*fields);
    ASSERT_TRUE(table_heap->InsertTuple(row, nullptr));
    if (row_values.find(row.GetRowId().Get()) != row_values.end()) {
      std::cout << row.GetRowId().Get() << std::endl;
      ASSERT_TRUE(false);
    } else {
      row_values.emplace(row.GetRowId().Get(), fields);
      size++;
    }
    delete[] characters;
  }

  ASSERT_EQ(row_nums, row_values.size());
  ASSERT_EQ(row_nums, size);
  for (auto row_kv : row_values) {
    size--;
    Row row(RowId(row_kv.first));
    table_heap->GetTuple(&row, nullptr);
    ASSERT_EQ(schema.get()->GetColumnCount(), row.GetFields().size());
    for (size_t j = 0; j < schema.get()->GetColumnCount(); j++) {
      ASSERT_EQ(CmpBool::kTrue, row.GetField(j)->CompareEquals(row_kv.second->at(j)));
    }
    // free spaces
    delete row_kv.second;
  }
  ASSERT_EQ(size, 0);
}

TEST(TableHeapTest, ReuseFreedSlotAfterApplyDelete) {
  remove(db_file_name.c_str());
  auto disk_mgr_ = new DiskManager(db_file_name);
  auto bpm_ = new BufferPoolManager(DEFAULT_BUFFER_POOL_SIZE, disk_mgr_);
  std::vector<Column *> columns = {new Column("id", TypeId::kTypeInt, 0, false, false),
                                   new Column("name", TypeId::kTypeChar, 64, 1, true, false),
                                   new Column("account", TypeId::kTypeFloat, 2, true, false)};
  auto schema = std::make_shared<Schema>(columns);
  TableHeap *table_heap = TableHeap::Create(bpm_, schema.get(), nullptr, nullptr, nullptr);

  std::vector<Field> fields_a = {Field(TypeId::kTypeInt, 1),
                                 Field(TypeId::kTypeChar, const_cast<char *>("alice"), 5, false),
                                 Field(TypeId::kTypeFloat, 1.0f)};
  std::vector<Field> fields_b = {Field(TypeId::kTypeInt, 2),
                                 Field(TypeId::kTypeChar, const_cast<char *>("bob"), 3, false),
                                 Field(TypeId::kTypeFloat, 2.0f)};
  std::vector<Field> fields_c = {Field(TypeId::kTypeInt, 3),
                                 Field(TypeId::kTypeChar, const_cast<char *>("carol"), 5, false),
                                 Field(TypeId::kTypeFloat, 3.0f)};

  Row row_a(fields_a);
  Row row_b(fields_b);
  Row row_c(fields_c);
  ASSERT_TRUE(table_heap->InsertTuple(row_a, nullptr));
  ASSERT_TRUE(table_heap->InsertTuple(row_b, nullptr));
  ASSERT_TRUE(table_heap->InsertTuple(row_c, nullptr));

  RowId freed_rid = row_b.GetRowId();
  ASSERT_TRUE(table_heap->MarkDelete(freed_rid, nullptr));
  table_heap->ApplyDelete(freed_rid, nullptr);

  std::vector<Field> fields_d = {Field(TypeId::kTypeInt, 4),
                                 Field(TypeId::kTypeChar, const_cast<char *>("dave"), 4, false),
                                 Field(TypeId::kTypeFloat, 4.0f)};
  Row row_d(fields_d);
  ASSERT_TRUE(table_heap->InsertTuple(row_d, nullptr));
  ASSERT_EQ(freed_rid, row_d.GetRowId());

  size_t live_tuple_count = 0;
  bool found_new_row = false;
  for (auto it = table_heap->Begin(nullptr); it != table_heap->End(); ++it) {
    live_tuple_count++;
    ASSERT_EQ(CmpBool::kFalse, it->GetField(0)->CompareEquals(Field(TypeId::kTypeInt, 2)));
    if (it->GetField(0)->CompareEquals(Field(TypeId::kTypeInt, 4)) == CmpBool::kTrue) {
      found_new_row = true;
    }
  }
  ASSERT_EQ(live_tuple_count, 3);
  ASSERT_TRUE(found_new_row);
}
