#include "catalog/indexes.h"

#include <algorithm>
#include <cstdio>
#include <fstream>

namespace {

class SimpleIndex : public Index {
 public:
  explicit SimpleIndex(index_id_t index_id, IndexSchema *key_schema)
      : Index(index_id, key_schema), file_name_("simple_index_" + std::to_string(index_id) + ".dat") {
    Load();
  }

  dberr_t InsertEntry(const Row &key, RowId row_id, Txn *txn) override {
    entries_.emplace_back(key, row_id);
    Persist();
    return DB_SUCCESS;
  }

  dberr_t RemoveEntry(const Row &key, RowId row_id, Txn *txn) override {
    auto iter = std::find_if(entries_.begin(), entries_.end(), [&](const auto &entry) {
      return entry.second.Get() == row_id.Get() && CompareRow(entry.first, key) == 0;
    });
    if (iter == entries_.end()) {
      return DB_KEY_NOT_FOUND;
    }
    entries_.erase(iter);
    Persist();
    return DB_SUCCESS;
  }

  dberr_t ScanKey(const Row &key, std::vector<RowId> &result, Txn *txn, string compare_operator = "=") override {
    bool found = false;
    for (auto &entry : entries_) {
      int cmp = CompareRow(entry.first, key);
      if (MatchCompare(cmp, compare_operator)) {
        result.push_back(entry.second);
        found = true;
      }
    }
    return found ? DB_SUCCESS : DB_KEY_NOT_FOUND;
  }

  dberr_t Destroy() override {
    entries_.clear();
    std::remove(file_name_.c_str());
    return DB_SUCCESS;
  }

 private:
  void Persist() {
    std::ofstream out(file_name_, std::ios::binary | std::ios::trunc);
    uint32_t count = static_cast<uint32_t>(entries_.size());
    out.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (auto &entry : entries_) {
      int64_t rid = entry.second.Get();
      uint32_t row_size = entry.first.GetSerializedSize(key_schema_);
      std::vector<char> buf(row_size);
      entry.first.SerializeTo(buf.data(), key_schema_);
      out.write(reinterpret_cast<const char *>(&rid), sizeof(rid));
      out.write(reinterpret_cast<const char *>(&row_size), sizeof(row_size));
      out.write(buf.data(), row_size);
    }
  }

  void Load() {
    std::ifstream in(file_name_, std::ios::binary);
    if (!in.is_open()) {
      return;
    }
    uint32_t count = 0;
    in.read(reinterpret_cast<char *>(&count), sizeof(count));
    for (uint32_t i = 0; i < count; i++) {
      int64_t rid = 0;
      uint32_t row_size = 0;
      in.read(reinterpret_cast<char *>(&rid), sizeof(rid));
      in.read(reinterpret_cast<char *>(&row_size), sizeof(row_size));
      std::vector<char> buf(row_size);
      in.read(buf.data(), row_size);
      Row row;
      row.DeserializeFrom(buf.data(), key_schema_);
      entries_.emplace_back(row, RowId(rid));
    }
  }

  static int CompareField(const Field *lhs, const Field *rhs) {
    if (lhs->CompareLessThan(*rhs) == CmpBool::kTrue) {
      return -1;
    }
    if (lhs->CompareGreaterThan(*rhs) == CmpBool::kTrue) {
      return 1;
    }
    return 0;
  }

  static int CompareRow(const Row &lhs, const Row &rhs) {
    auto count = std::min(lhs.GetFieldCount(), rhs.GetFieldCount());
    for (size_t i = 0; i < count; i++) {
      int cmp = CompareField(lhs.GetField(static_cast<uint32_t>(i)), rhs.GetField(static_cast<uint32_t>(i)));
      if (cmp != 0) {
        return cmp;
      }
    }
    if (lhs.GetFieldCount() == rhs.GetFieldCount()) {
      return 0;
    }
    return lhs.GetFieldCount() < rhs.GetFieldCount() ? -1 : 1;
  }

  static bool MatchCompare(int cmp, const string &compare_operator) {
    if (compare_operator == "=") {
      return cmp == 0;
    }
    if (compare_operator == "<>") {
      return cmp != 0;
    }
    if (compare_operator == ">") {
      return cmp > 0;
    }
    if (compare_operator == ">=") {
      return cmp >= 0;
    }
    if (compare_operator == "<") {
      return cmp < 0;
    }
    if (compare_operator == "<=") {
      return cmp <= 0;
    }
    return false;
  }

 private:
  std::string file_name_;
  std::vector<std::pair<Row, RowId>> entries_;
};

}  // namespace

IndexMetadata::IndexMetadata(const index_id_t index_id, const std::string &index_name, const table_id_t table_id,
                             const std::vector<uint32_t> &key_map)
    : index_id_(index_id), index_name_(index_name), table_id_(table_id), key_map_(key_map) {}

IndexMetadata *IndexMetadata::Create(const index_id_t index_id, const string &index_name, const table_id_t table_id,
                                     const vector<uint32_t> &key_map) {
  return new IndexMetadata(index_id, index_name, table_id, key_map);
}

uint32_t IndexMetadata::SerializeTo(char *buf) const {
  char *p = buf;
  uint32_t ofs = GetSerializedSize();
  ASSERT(ofs <= PAGE_SIZE, "Failed to serialize index info.");
  // magic num
  MACH_WRITE_UINT32(buf, INDEX_METADATA_MAGIC_NUM);
  buf += 4;
  // index id
  MACH_WRITE_TO(index_id_t, buf, index_id_);
  buf += 4;
  // index name
  MACH_WRITE_UINT32(buf, index_name_.length());
  buf += 4;
  MACH_WRITE_STRING(buf, index_name_);
  buf += index_name_.length();
  // table id
  MACH_WRITE_TO(table_id_t, buf, table_id_);
  buf += 4;
  // key count
  MACH_WRITE_UINT32(buf, key_map_.size());
  buf += 4;
  // key mapping in table
  for (auto &col_index : key_map_) {
    MACH_WRITE_UINT32(buf, col_index);
    buf += 4;
  }
  ASSERT(buf - p == ofs, "Unexpected serialize size.");
  return ofs;
}

/**
 * TODO: Student Implement
 */
uint32_t IndexMetadata::GetSerializedSize() const {
  return 4 + 4 + MACH_STR_SERIALIZED_SIZE(index_name_) + 4 + 4 + 4 * static_cast<uint32_t>(key_map_.size());
}

uint32_t IndexMetadata::DeserializeFrom(char *buf, IndexMetadata *&index_meta) {
  if (index_meta != nullptr) {
    LOG(WARNING) << "Pointer object index info is not null in table info deserialize." << std::endl;
  }
  char *p = buf;
  // magic num
  uint32_t magic_num = MACH_READ_UINT32(buf);
  buf += 4;
  ASSERT(magic_num == INDEX_METADATA_MAGIC_NUM, "Failed to deserialize index info.");
  // index id
  index_id_t index_id = MACH_READ_FROM(index_id_t, buf);
  buf += 4;
  // index name
  uint32_t len = MACH_READ_UINT32(buf);
  buf += 4;
  std::string index_name(buf, len);
  buf += len;
  // table id
  table_id_t table_id = MACH_READ_FROM(table_id_t, buf);
  buf += 4;
  // index key count
  uint32_t index_key_count = MACH_READ_UINT32(buf);
  buf += 4;
  // key mapping in table
  std::vector<uint32_t> key_map;
  for (uint32_t i = 0; i < index_key_count; i++) {
    uint32_t key_index = MACH_READ_UINT32(buf);
    buf += 4;
    key_map.push_back(key_index);
  }
  // allocate space for index meta data
  index_meta = new IndexMetadata(index_id, index_name, table_id, key_map);
  return buf - p;
}

Index *IndexInfo::CreateIndex(BufferPoolManager *buffer_pool_manager, const string &index_type) {
  return new SimpleIndex(meta_data_->index_id_, key_schema_);
}
