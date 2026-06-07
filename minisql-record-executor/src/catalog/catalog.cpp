#include "catalog/catalog.h"

#include <utility>

void CatalogMeta::SerializeTo(char *buf) const {
  ASSERT(GetSerializedSize() <= PAGE_SIZE, "Failed to serialize catalog metadata to disk.");
  MACH_WRITE_UINT32(buf, CATALOG_METADATA_MAGIC_NUM);
  buf += 4;
  MACH_WRITE_UINT32(buf, table_meta_pages_.size());
  buf += 4;
  MACH_WRITE_UINT32(buf, index_meta_pages_.size());
  buf += 4;
  for (auto iter : table_meta_pages_) {
    MACH_WRITE_TO(table_id_t, buf, iter.first);
    buf += 4;
    MACH_WRITE_TO(page_id_t, buf, iter.second);
    buf += 4;
  }
  for (auto iter : index_meta_pages_) {
    MACH_WRITE_TO(index_id_t, buf, iter.first);
    buf += 4;
    MACH_WRITE_TO(page_id_t, buf, iter.second);
    buf += 4;
  }
}

CatalogMeta *CatalogMeta::DeserializeFrom(char *buf) {
  // check valid
  uint32_t magic_num = MACH_READ_UINT32(buf);
  buf += 4;
  ASSERT(magic_num == CATALOG_METADATA_MAGIC_NUM, "Failed to deserialize catalog metadata from disk.");
  // get table and index nums
  uint32_t table_nums = MACH_READ_UINT32(buf);
  buf += 4;
  uint32_t index_nums = MACH_READ_UINT32(buf);
  buf += 4;
  // create metadata and read value
  CatalogMeta *meta = new CatalogMeta();
  for (uint32_t i = 0; i < table_nums; i++) {
    auto table_id = MACH_READ_FROM(table_id_t, buf);
    buf += 4;
    auto table_heap_page_id = MACH_READ_FROM(page_id_t, buf);
    buf += 4;
    meta->table_meta_pages_.emplace(table_id, table_heap_page_id);
  }
  for (uint32_t i = 0; i < index_nums; i++) {
    auto index_id = MACH_READ_FROM(index_id_t, buf);
    buf += 4;
    auto index_page_id = MACH_READ_FROM(page_id_t, buf);
    buf += 4;
    meta->index_meta_pages_.emplace(index_id, index_page_id);
  }
  return meta;
}

/**
 * TODO: Student Implement
 */
uint32_t CatalogMeta::GetSerializedSize() const {
  return 12 + 8 * static_cast<uint32_t>(table_meta_pages_.size() + index_meta_pages_.size());
}

CatalogMeta::CatalogMeta() {}

/**
 * TODO: Student Implement
 */
CatalogManager::CatalogManager(BufferPoolManager *buffer_pool_manager, LockManager *lock_manager,
                               LogManager *log_manager, bool init)
    : buffer_pool_manager_(buffer_pool_manager), lock_manager_(lock_manager), log_manager_(log_manager) {
  if (init) {
    catalog_meta_ = CatalogMeta::NewInstance();
    next_table_id_ = 0;
    next_index_id_ = 0;
    FlushCatalogMetaPage();
    return;
  }

  auto *page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  ASSERT(page != nullptr, "Failed to load catalog meta page.");
  catalog_meta_ = CatalogMeta::DeserializeFrom(page->GetData());
  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, false);

  for (auto &entry : *catalog_meta_->GetTableMetaPages()) {
    LoadTable(entry.first, entry.second);
  }
  for (auto &entry : *catalog_meta_->GetIndexMetaPages()) {
    LoadIndex(entry.first, entry.second);
  }
  next_table_id_ = catalog_meta_->GetNextTableId();
  next_index_id_ = catalog_meta_->GetNextIndexId();
}

CatalogManager::~CatalogManager() {
  FlushCatalogMetaPage();
  delete catalog_meta_;
  for (auto iter : tables_) {
    delete iter.second;
  }
  for (auto iter : indexes_) {
    delete iter.second;
  }
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::CreateTable(const string &table_name, TableSchema *schema, Txn *txn, TableInfo *&table_info) {
  if (table_names_.find(table_name) != table_names_.end()) {
    return DB_TABLE_ALREADY_EXIST;
  }

  page_id_t meta_page_id = INVALID_PAGE_ID;
  auto *meta_page = buffer_pool_manager_->NewPage(meta_page_id);
  if (meta_page == nullptr) {
    return DB_FAILED;
  }

  auto table_id = next_table_id_.load();
  auto *table_schema = Schema::DeepCopySchema(schema);
  auto *table_heap = TableHeap::Create(buffer_pool_manager_, table_schema, txn, log_manager_, lock_manager_);
  auto *table_meta = TableMetadata::Create(table_id, table_name, table_heap->GetFirstPageId(), table_schema);
  table_meta->SerializeTo(meta_page->GetData());
  buffer_pool_manager_->UnpinPage(meta_page_id, true);

  table_info = TableInfo::Create();
  table_info->Init(table_meta, table_heap);
  table_names_[table_name] = table_id;
  tables_[table_id] = table_info;
  catalog_meta_->table_meta_pages_[table_id] = meta_page_id;
  next_table_id_++;
  FlushCatalogMetaPage();
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetTable(const string &table_name, TableInfo *&table_info) {
  auto iter = table_names_.find(table_name);
  if (iter == table_names_.end()) {
    table_info = nullptr;
    return DB_TABLE_NOT_EXIST;
  }
  table_info = tables_[iter->second];
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetTables(vector<TableInfo *> &tables) const {
  tables.clear();
  for (auto &entry : tables_) {
    tables.push_back(entry.second);
  }
  return tables.empty() ? DB_FAILED : DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::CreateIndex(const std::string &table_name, const string &index_name,
                                    const std::vector<std::string> &index_keys, Txn *txn, IndexInfo *&index_info,
                                    const string &index_type) {
  TableInfo *table_info = nullptr;
  auto table_result = GetTable(table_name, table_info);
  if (table_result != DB_SUCCESS) {
    return table_result;
  }
  if (index_names_[table_name].find(index_name) != index_names_[table_name].end()) {
    return DB_INDEX_ALREADY_EXIST;
  }

  std::vector<uint32_t> key_map;
  for (auto &key_name : index_keys) {
    uint32_t column_index = 0;
    if (table_info->GetSchema()->GetColumnIndex(key_name, column_index) != DB_SUCCESS) {
      return DB_COLUMN_NAME_NOT_EXIST;
    }
    key_map.push_back(column_index);
  }

  page_id_t meta_page_id = INVALID_PAGE_ID;
  auto *meta_page = buffer_pool_manager_->NewPage(meta_page_id);
  if (meta_page == nullptr) {
    return DB_FAILED;
  }

  auto index_id = next_index_id_.load();
  auto *index_meta = IndexMetadata::Create(index_id, index_name, table_info->GetTableId(), key_map);
  index_meta->SerializeTo(meta_page->GetData());
  buffer_pool_manager_->UnpinPage(meta_page_id, true);
  std::remove(("simple_index_" + std::to_string(index_id) + ".dat").c_str());

  index_info = IndexInfo::Create();
  index_info->Init(index_meta, table_info, buffer_pool_manager_);
  // 新建索引时，把表里已有记录补进索引。
  for (auto iter = table_info->GetTableHeap()->Begin(nullptr); iter != table_info->GetTableHeap()->End(); ++iter) {
    Row key_row;
    (*iter).GetKeyFromRow(table_info->GetSchema(), index_info->GetIndexKeySchema(), key_row);
    index_info->GetIndex()->InsertEntry(key_row, (*iter).GetRowId(), txn);
  }
  indexes_[index_id] = index_info;
  index_names_[table_name][index_name] = index_id;
  catalog_meta_->index_meta_pages_[index_id] = meta_page_id;
  next_index_id_++;
  FlushCatalogMetaPage();
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetIndex(const std::string &table_name, const std::string &index_name,
                                 IndexInfo *&index_info) const {
  auto table_iter = index_names_.find(table_name);
  if (table_iter == index_names_.end()) {
    index_info = nullptr;
    return DB_INDEX_NOT_FOUND;
  }
  auto index_iter = table_iter->second.find(index_name);
  if (index_iter == table_iter->second.end()) {
    index_info = nullptr;
    return DB_INDEX_NOT_FOUND;
  }
  index_info = indexes_.at(index_iter->second);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetTableIndexes(const std::string &table_name, std::vector<IndexInfo *> &indexes) const {
  indexes.clear();
  auto table_iter = index_names_.find(table_name);
  if (table_iter == index_names_.end()) {
    return table_names_.find(table_name) == table_names_.end() ? DB_TABLE_NOT_EXIST : DB_SUCCESS;
  }
  for (auto &entry : table_iter->second) {
    indexes.push_back(indexes_.at(entry.second));
  }
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::DropTable(const string &table_name) {
  auto iter = table_names_.find(table_name);
  if (iter == table_names_.end()) {
    return DB_TABLE_NOT_EXIST;
  }

  std::vector<std::string> index_names;
  for (auto &entry : index_names_[table_name]) {
    index_names.push_back(entry.first);
  }
  for (auto &index_name : index_names) {
    DropIndex(table_name, index_name);
  }
  return DropTable(iter->second);
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::DropIndex(const string &table_name, const string &index_name) {
  auto table_iter = index_names_.find(table_name);
  if (table_iter == index_names_.end()) {
    return DB_INDEX_NOT_FOUND;
  }
  auto index_iter = table_iter->second.find(index_name);
  if (index_iter == table_iter->second.end()) {
    return DB_INDEX_NOT_FOUND;
  }

  auto index_id = index_iter->second;
  auto info_iter = indexes_.find(index_id);
  if (info_iter != indexes_.end()) {
    info_iter->second->GetIndex()->Destroy();
    delete info_iter->second;
    indexes_.erase(info_iter);
  }
  table_iter->second.erase(index_iter);
  if (table_iter->second.empty()) {
    index_names_.erase(table_iter);
  }
  catalog_meta_->DeleteIndexMetaPage(buffer_pool_manager_, index_id);
  FlushCatalogMetaPage();
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::FlushCatalogMetaPage() const {
  auto *page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  if (page == nullptr) {
    return DB_FAILED;
  }
  memset(page->GetData(), 0, PAGE_SIZE);
  catalog_meta_->SerializeTo(page->GetData());
  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, true);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::LoadTable(const table_id_t table_id, const page_id_t page_id) {
  auto *page = buffer_pool_manager_->FetchPage(page_id);
  if (page == nullptr) {
    return DB_FAILED;
  }
  TableMetadata *table_meta = nullptr;
  TableMetadata::DeserializeFrom(page->GetData(), table_meta);
  buffer_pool_manager_->UnpinPage(page_id, false);

  auto *table_heap = TableHeap::Create(buffer_pool_manager_, table_meta->GetFirstPageId(), table_meta->GetSchema(),
                                       log_manager_, lock_manager_);
  auto *table_info = TableInfo::Create();
  table_info->Init(table_meta, table_heap);
  table_names_[table_meta->GetTableName()] = table_id;
  tables_[table_id] = table_info;
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::LoadIndex(const index_id_t index_id, const page_id_t page_id) {
  auto *page = buffer_pool_manager_->FetchPage(page_id);
  if (page == nullptr) {
    return DB_FAILED;
  }
  IndexMetadata *index_meta = nullptr;
  IndexMetadata::DeserializeFrom(page->GetData(), index_meta);
  buffer_pool_manager_->UnpinPage(page_id, false);

  TableInfo *table_info = nullptr;
  if (GetTable(index_meta->GetTableId(), table_info) != DB_SUCCESS) {
    delete index_meta;
    return DB_TABLE_NOT_EXIST;
  }

  auto *index_info = IndexInfo::Create();
  index_info->Init(index_meta, table_info, buffer_pool_manager_);
  indexes_[index_id] = index_info;
  index_names_[table_info->GetTableName()][index_meta->GetIndexName()] = index_id;
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetTable(const table_id_t table_id, TableInfo *&table_info) {
  auto iter = tables_.find(table_id);
  if (iter == tables_.end()) {
    table_info = nullptr;
    return DB_TABLE_NOT_EXIST;
  }
  table_info = iter->second;
  return DB_SUCCESS;
}

dberr_t CatalogManager::DropTable(table_id_t table_id) {
  auto iter = tables_.find(table_id);
  if (iter == tables_.end()) {
    return DB_TABLE_NOT_EXIST;
  }

  auto *table_info = iter->second;
  string table_name = table_info->GetTableName();
  page_id_t meta_page_id = catalog_meta_->table_meta_pages_[table_id];
  table_info->GetTableHeap()->DeleteTable();
  buffer_pool_manager_->DeletePage(meta_page_id);
  delete table_info;

  tables_.erase(table_id);
  table_names_.erase(table_name);
  index_names_.erase(table_name);
  catalog_meta_->table_meta_pages_.erase(table_id);
  FlushCatalogMetaPage();
  return DB_SUCCESS;
}
