#include "catalog/catalog.h"

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
 * Calculate the serialized size of CatalogMeta.
 * Format: | magic_num(4) | table_count(4) | index_count(4) |
 *         | for each table: table_id(4) + page_id(4) |
 *         | for each index: index_id(4) + page_id(4) |
 */
uint32_t CatalogMeta::GetSerializedSize() const {
  // 4 (magic) + 4 (table count) + 4 (index count)
  // + table_meta_pages_.size() * (4 + 4)  [table_id + page_id]
  // + index_meta_pages_.size() * (4 + 4)  [index_id + page_id]
  return 4 + 4 + 4 + table_meta_pages_.size() * 8 + index_meta_pages_.size() * 8;
}

CatalogMeta::CatalogMeta() {}

/**
 * CatalogManager constructor.
 * If init == true:  create a brand-new catalog (empty tables/indexes).
 * If init == false: load existing catalog from the catalog meta page on disk,
 *                   then load all tables and indexes into memory.
 */
CatalogManager::CatalogManager(BufferPoolManager *buffer_pool_manager, LockManager *lock_manager,
                               LogManager *log_manager, bool init)
    : buffer_pool_manager_(buffer_pool_manager), lock_manager_(lock_manager), log_manager_(log_manager) {
  if (init) {
    // Initialize a fresh catalog
    catalog_meta_ = CatalogMeta::NewInstance();
    next_table_id_ = catalog_meta_->GetNextTableId();  // = 0
    next_index_id_ = catalog_meta_->GetNextIndexId();  // = 0
  } else {
    // Load catalog meta page from disk
    Page *meta_page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
    ASSERT(meta_page != nullptr, "Failed to fetch catalog meta page.");
    catalog_meta_ = CatalogMeta::DeserializeFrom(meta_page->GetData());
    buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, false);

    next_table_id_ = catalog_meta_->GetNextTableId();
    next_index_id_ = catalog_meta_->GetNextIndexId();

    // Load all tables registered in the catalog meta
    for (auto &[table_id, page_id] : *catalog_meta_->GetTableMetaPages()) {
      LoadTable(table_id, page_id);
    }

    // Load all indexes registered in the catalog meta
    for (auto &[index_id, page_id] : *catalog_meta_->GetIndexMetaPages()) {
      LoadIndex(index_id, page_id);
    }
  }
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
 * Create a new table.
 * 1. Verify the table name does not already exist.
 * 2. Allocate a new table id.
 * 3. Deep-copy the schema so the catalog owns its own copy.
 * 4. Create the TableHeap (allocates the underlying storage pages).
 * 5. Create TableMetadata and TableInfo, register in catalog maps.
 * 6. Persist the updated catalog meta page.
 */
dberr_t CatalogManager::CreateTable(const string &table_name, TableSchema *schema, Txn *txn, TableInfo *&table_info) {
  // 1. Check for duplicate table name
  if (table_names_.find(table_name) != table_names_.end()) {
    return DB_TABLE_ALREADY_EXIST;
  }

  // 2. Get the next table id
  table_id_t table_id = next_table_id_++;

  // 3. Deep-copy schema (catalog takes ownership)
  TableSchema *copied_schema = Schema::DeepCopySchema(schema);

  // 4. Create the TableHeap — this allocates the root page for table data storage
  TableHeap *table_heap = TableHeap::Create(buffer_pool_manager_, copied_schema, txn, log_manager_, lock_manager_);

  // 5. Create TableMetadata (stores what we need to reconstruct the table)
  TableMetadata *table_meta = TableMetadata::Create(table_id, table_name, table_heap->GetFirstPageId(), copied_schema);

  // 6. Allocate a SEPARATE page for the serialized TableMetadata
  page_id_t meta_page_id;
  Page *meta_page = buffer_pool_manager_->NewPage(meta_page_id);
  if (meta_page == nullptr) {
    delete table_meta;
    delete table_heap;
    return DB_FAILED;
  }
  table_meta->SerializeTo(meta_page->GetData());
  buffer_pool_manager_->UnpinPage(meta_page_id, true);

  // 7. Create TableInfo
  table_info = TableInfo::Create();
  table_info->Init(table_meta, table_heap);

  // 8. Register in catalog maps (store the metadata page id, NOT the data page id)
  table_names_[table_name] = table_id;
  tables_[table_id] = table_info;
  catalog_meta_->table_meta_pages_[table_id] = meta_page_id;

  // 9. Persist catalog meta to disk
  FlushCatalogMetaPage();

  return DB_SUCCESS;
}

/**
 * Lookup a table by name.
 */
dberr_t CatalogManager::GetTable(const string &table_name, TableInfo *&table_info) {
  auto it = table_names_.find(table_name);
  if (it == table_names_.end()) {
    return DB_TABLE_NOT_EXIST;
  }
  return GetTable(it->second, table_info);
}

/**
 * Return all registered tables.
 */
dberr_t CatalogManager::GetTables(vector<TableInfo *> &tables) const {
  for (auto &pair : tables_) {
    tables.push_back(pair.second);
  }
  return DB_SUCCESS;
}

/**
 * Create a new index on a table.
 * 1. Verify the table exists.
 * 2. Verify the index name isn't already used for that table.
 * 3. Resolve each index key column name to a column index in the table schema.
 * 4. Create IndexMetadata, allocate a page for it, serialize it.
 * 5. Create IndexInfo and call Init to build the B+ tree.
 * 6. Register in catalog maps and persist.
 */
dberr_t CatalogManager::CreateIndex(const std::string &table_name, const string &index_name,
                                    const std::vector<std::string> &index_keys, Txn *txn, IndexInfo *&index_info,
                                    const string &index_type) {
  // 1. Verify table exists
  auto table_it = table_names_.find(table_name);
  if (table_it == table_names_.end()) {
    return DB_TABLE_NOT_EXIST;
  }

  table_id_t table_id = table_it->second;
  TableInfo *table_info = tables_[table_id];

  // 2. Check for duplicate index name on this table
  auto &table_indexes = index_names_[table_name];
  if (table_indexes.find(index_name) != table_indexes.end()) {
    return DB_INDEX_ALREADY_EXIST;
  }

  // 3. Map index key column names to column indices in the table schema
  Schema *table_schema = table_info->GetSchema();
  std::vector<uint32_t> key_map;
  for (const auto &col_name : index_keys) {
    uint32_t col_idx;
    if (table_schema->GetColumnIndex(col_name, col_idx) != DB_SUCCESS) {
      return DB_COLUMN_NAME_NOT_EXIST;
    }
    key_map.push_back(col_idx);
  }

  // 4. Create IndexMetadata
  index_id_t index_id = next_index_id_++;
  IndexMetadata *index_meta = IndexMetadata::Create(index_id, index_name, table_id, key_map);

  // 5. Create IndexInfo and initialize it (builds the B+ tree)
  index_info = IndexInfo::Create();
  index_info->Init(index_meta, table_info, buffer_pool_manager_);

  // 6. Allocate a page for the serialized IndexMetadata and persist it
  page_id_t meta_page_id;
  Page *meta_page = buffer_pool_manager_->NewPage(meta_page_id);
  if (meta_page == nullptr) {
    delete index_info;
    return DB_FAILED;
  }
  index_meta->SerializeTo(meta_page->GetData());
  buffer_pool_manager_->UnpinPage(meta_page_id, true);

  // 7. Register in catalog maps
  indexes_[index_id] = index_info;
  table_indexes[index_name] = index_id;
  catalog_meta_->index_meta_pages_[index_id] = meta_page_id;

  // 8. Persist catalog meta
  FlushCatalogMetaPage();

  return DB_SUCCESS;
}

/**
 * Lookup an index by table name and index name.
 */
dberr_t CatalogManager::GetIndex(const std::string &table_name, const std::string &index_name,
                                 IndexInfo *&index_info) const {
  auto table_it = index_names_.find(table_name);
  if (table_it == index_names_.end()) {
    return DB_INDEX_NOT_FOUND;
  }

  auto idx_it = table_it->second.find(index_name);
  if (idx_it == table_it->second.end()) {
    return DB_INDEX_NOT_FOUND;
  }

  index_info = indexes_.at(idx_it->second);
  return DB_SUCCESS;
}

/**
 * Return all indexes associated with a given table.
 */
dberr_t CatalogManager::GetTableIndexes(const std::string &table_name, std::vector<IndexInfo *> &indexes) const {
  auto table_it = index_names_.find(table_name);
  if (table_it == index_names_.end()) {
    // No indexes for this table — return empty list, not an error
    return DB_SUCCESS;
  }

  for (const auto &pair : table_it->second) {
    indexes.push_back(indexes_.at(pair.second));
  }
  return DB_SUCCESS;
}

/**
 * Drop a table by name.
 * Also drops all indexes associated with the table.
 */
dberr_t CatalogManager::DropTable(const string &table_name) {
  auto it = table_names_.find(table_name);
  if (it == table_names_.end()) {
    return DB_TABLE_NOT_EXIST;
  }
  return DropTable(it->second);
}

/**
 * Drop a table by id.
 * 1. Drop all indexes on this table first.
 * 2. Free the table heap pages on disk.
 * 3. Remove from catalog maps and persist.
 */
dberr_t CatalogManager::DropTable(table_id_t table_id) {
  auto it = tables_.find(table_id);
  if (it == tables_.end()) {
    return DB_TABLE_NOT_EXIST;
  }

  TableInfo *table_info = it->second;
  std::string table_name = table_info->GetTableName();

  // 1. Drop all indexes on this table
  auto idx_it = index_names_.find(table_name);
  if (idx_it != index_names_.end()) {
    // Collect index names first (DropIndex modifies the map)
    std::vector<std::string> names_to_drop;
    for (const auto &pair : idx_it->second) {
      names_to_drop.push_back(pair.first);
    }
    for (const auto &idx_name : names_to_drop) {
      DropIndex(table_name, idx_name);
    }
  }

  // 2. Free the table heap pages (data pages)
  table_info->GetTableHeap()->DeleteTable();

  // 3. Delete the table metadata page from disk
  page_id_t meta_page_id = catalog_meta_->table_meta_pages_[table_id];
  buffer_pool_manager_->DeletePage(meta_page_id);
  catalog_meta_->table_meta_pages_.erase(table_id);

  // 4. Remove from catalog maps
  table_names_.erase(table_name);
  tables_.erase(it);

  // 5. Persist changes
  FlushCatalogMetaPage();

  delete table_info;
  return DB_SUCCESS;
}

/**
 * Drop an index by table name and index name.
 * 1. Destroy the underlying B+ tree.
 * 2. Delete the index metadata page.
 * 3. Remove from catalog maps and persist.
 */
dberr_t CatalogManager::DropIndex(const string &table_name, const string &index_name) {
  // 1. Find the index
  auto table_it = index_names_.find(table_name);
  if (table_it == index_names_.end()) {
    return DB_INDEX_NOT_FOUND;
  }

  auto idx_it = table_it->second.find(index_name);
  if (idx_it == table_it->second.end()) {
    return DB_INDEX_NOT_FOUND;
  }

  index_id_t index_id = idx_it->second;
  IndexInfo *index_info = indexes_[index_id];

  // 2. Destroy the B+ tree (free index pages on disk)
  index_info->GetIndex()->Destroy();

  // 3. Delete the index metadata page from disk and catalog meta
  catalog_meta_->DeleteIndexMetaPage(buffer_pool_manager_, index_id);

  // 4. Remove from catalog maps
  indexes_.erase(index_id);
  table_it->second.erase(idx_it);
  if (table_it->second.empty()) {
    index_names_.erase(table_it);
  }

  // 5. Persist changes
  FlushCatalogMetaPage();

  delete index_info;
  return DB_SUCCESS;
}

/**
 * Serialize the in-memory CatalogMeta to the catalog meta page on disk.
 */
dberr_t CatalogManager::FlushCatalogMetaPage() const {
  char buf[PAGE_SIZE];
  memset(buf, 0, PAGE_SIZE);
  catalog_meta_->SerializeTo(buf);

  Page *page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  if (page == nullptr) {
    return DB_FAILED;
  }
  memcpy(page->GetData(), buf, PAGE_SIZE);
  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, true);

  return DB_SUCCESS;
}

/**
 * Load a table from disk into memory.
 * Reads the table metadata from the given page, creates the TableHeap and
 * TableInfo objects, and registers them in the in-memory catalog maps.
 */
dberr_t CatalogManager::LoadTable(const table_id_t table_id, const page_id_t page_id) {
  Page *page = buffer_pool_manager_->FetchPage(page_id);
  if (page == nullptr) {
    return DB_FAILED;
  }

  // Deserialize TableMetadata from the metadata page
  TableMetadata *table_meta = nullptr;
  TableMetadata::DeserializeFrom(page->GetData(), table_meta);
  ASSERT(table_meta != nullptr, "Failed to deserialize table metadata.");
  ASSERT(table_meta->GetTableId() == table_id, "Table id mismatch.");

  // Create TableHeap on the existing table data pages (root page id is stored in metadata)
  TableHeap *table_heap = TableHeap::Create(buffer_pool_manager_, table_meta->GetFirstPageId(),
                                            table_meta->GetSchema(), log_manager_, lock_manager_);

  // Create TableInfo
  TableInfo *table_info = TableInfo::Create();
  table_info->Init(table_meta, table_heap);

  // Register in in-memory maps
  table_names_[table_meta->GetTableName()] = table_id;
  tables_[table_id] = table_info;

  buffer_pool_manager_->UnpinPage(page_id, false);
  return DB_SUCCESS;
}

/**
 * Load an index from disk into memory.
 * Reads the IndexMetadata from the given page, creates the IndexInfo, and
 * initializes the B+ tree. Registers in the in-memory catalog maps.
 */
dberr_t CatalogManager::LoadIndex(const index_id_t index_id, const page_id_t page_id) {
  Page *page = buffer_pool_manager_->FetchPage(page_id);
  if (page == nullptr) {
    return DB_FAILED;
  }

  // Deserialize IndexMetadata from the page
  IndexMetadata *index_meta = nullptr;
  IndexMetadata::DeserializeFrom(page->GetData(), index_meta);
  ASSERT(index_meta != nullptr, "Failed to deserialize index metadata.");
  ASSERT(index_meta->GetIndexId() == index_id, "Index id mismatch.");

  // Get the associated table
  TableInfo *table_info = nullptr;
  dberr_t err = GetTable(index_meta->GetTableId(), table_info);
  ASSERT(err == DB_SUCCESS, "Table not found for index during load.");

  // Create IndexInfo and initialize (re-opens the B+ tree)
  IndexInfo *index_info = IndexInfo::Create();
  index_info->Init(index_meta, table_info, buffer_pool_manager_);

  // Register in in-memory maps
  indexes_[index_id] = index_info;
  index_names_[table_info->GetTableName()][index_meta->GetIndexName()] = index_id;

  buffer_pool_manager_->UnpinPage(page_id, false);
  return DB_SUCCESS;
}

/**
 * Lookup a table by table id.
 */
dberr_t CatalogManager::GetTable(const table_id_t table_id, TableInfo *&table_info) {
  auto it = tables_.find(table_id);
  if (it == tables_.end()) {
    return DB_TABLE_NOT_EXIST;
  }
  table_info = it->second;
  return DB_SUCCESS;
}
