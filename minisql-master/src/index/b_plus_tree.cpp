#include "index/b_plus_tree.h"

#include <string>

#include "glog/logging.h"
#include "index/basic_comparator.h"
#include "index/generic_key.h"
#include "page/index_roots_page.h"

BPlusTree::BPlusTree(index_id_t index_id, BufferPoolManager *buffer_pool_manager, const KeyManager &KM,
                     int leaf_max_size, int internal_max_size)
    : index_id_(index_id),
      buffer_pool_manager_(buffer_pool_manager),
      processor_(KM),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {
  int key_size = processor_.GetKeySize();
  if (leaf_max_size_ == UNDEFINED_SIZE) {
    leaf_max_size_ = (PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / (key_size + static_cast<int>(sizeof(RowId))) - 1;
  }
  if (internal_max_size_ == UNDEFINED_SIZE) {
    internal_max_size_ =
        (PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (key_size + static_cast<int>(sizeof(page_id_t))) - 1;
  }
  Page *roots_page = buffer_pool_manager_->FetchPage(INDEX_ROOTS_PAGE_ID);
  ASSERT(roots_page != nullptr, "Failed to fetch index roots page.");
  auto roots = reinterpret_cast<IndexRootsPage *>(roots_page->GetData());
  if (!roots->GetRootId(index_id_, &root_page_id_)) {
    root_page_id_ = INVALID_PAGE_ID;
  }
  buffer_pool_manager_->UnpinPage(INDEX_ROOTS_PAGE_ID, false);
}

void BPlusTree::Destroy(page_id_t current_page_id) {
  if (current_page_id == INVALID_PAGE_ID) {
    current_page_id = root_page_id_;
  }
  if (current_page_id == INVALID_PAGE_ID) {
    return;
  }
  Page *page = buffer_pool_manager_->FetchPage(current_page_id);
  if (page == nullptr) {
    return;
  }
  auto node = reinterpret_cast<BPlusTreePage *>(page->GetData());
  if (!node->IsLeafPage()) {
    auto internal = reinterpret_cast<InternalPage *>(node);
    for (int i = 0; i < internal->GetSize(); i++) {
      Destroy(internal->ValueAt(i));
    }
  }
  buffer_pool_manager_->UnpinPage(current_page_id, false);
  buffer_pool_manager_->DeletePage(current_page_id);
  if (current_page_id == root_page_id_) {
    root_page_id_ = INVALID_PAGE_ID;
    Page *roots_page = buffer_pool_manager_->FetchPage(INDEX_ROOTS_PAGE_ID);
    if (roots_page != nullptr) {
      reinterpret_cast<IndexRootsPage *>(roots_page->GetData())->Delete(index_id_);
      buffer_pool_manager_->UnpinPage(INDEX_ROOTS_PAGE_ID, true);
    }
  }
}

/*
 * Helper function to decide whether current b+tree is empty
 */
bool BPlusTree::IsEmpty() const {
  return root_page_id_ == INVALID_PAGE_ID;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
bool BPlusTree::GetValue(const GenericKey *key, std::vector<RowId> &result, Txn *transaction) {
  if (IsEmpty()) {
    return false;
  }
  Page *leaf_page = FindLeafPage(key);
  if (leaf_page == nullptr) {
    return false;
  }
  auto leaf = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  RowId value;
  bool found = leaf->Lookup(key, value, processor_);
  if (found) {
    result.emplace_back(value);
  }
  buffer_pool_manager_->UnpinPage(leaf->GetPageId(), false);
  return found;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
bool BPlusTree::Insert(GenericKey *key, const RowId &value, Txn *transaction) {
  if (IsEmpty()) {
    StartNewTree(key, value);
    return true;
  }
  return InsertIntoLeaf(key, value, transaction);
}
/*
 * Insert constant key & value pair into an empty tree
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then update b+
 * tree's root page id and insert entry directly into leaf page.
 */
void BPlusTree::StartNewTree(GenericKey *key, const RowId &value) {
  page_id_t page_id;
  Page *page = buffer_pool_manager_->NewPage(page_id);
  ASSERT(page != nullptr, "Failed to allocate new root page.");
  auto root = reinterpret_cast<LeafPage *>(page->GetData());
  root->Init(page_id, INVALID_PAGE_ID, processor_.GetKeySize(), leaf_max_size_);
  root->Insert(key, value, processor_);
  root_page_id_ = page_id;
  buffer_pool_manager_->UnpinPage(page_id, true);
  UpdateRootPageId(1);
}

/*
 * Insert constant key & value pair into leaf page
 * User needs to first find the right leaf page as insertion target, then look
 * through leaf page to see whether insert key exist or not. If exist, return
 * immediately, otherwise insert entry. Remember to deal with split if necessary.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
bool BPlusTree::InsertIntoLeaf(GenericKey *key, const RowId &value, Txn *transaction) {
  Page *leaf_page = FindLeafPage(key);
  ASSERT(leaf_page != nullptr, "Failed to find leaf page.");
  auto leaf = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  RowId old_value;
  if (leaf->Lookup(key, old_value, processor_)) {
    buffer_pool_manager_->UnpinPage(leaf->GetPageId(), false);
    return false;
  }
  leaf->Insert(key, value, processor_);
  if (leaf->GetSize() <= leaf->GetMaxSize()) {
    buffer_pool_manager_->UnpinPage(leaf->GetPageId(), true);
    return true;
  }
  LeafPage *new_leaf = Split(leaf, transaction);
  InsertIntoParent(leaf, new_leaf->KeyAt(0), new_leaf, transaction);
  buffer_pool_manager_->UnpinPage(leaf->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(new_leaf->GetPageId(), true);
  return true;
}

/*
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then move half
 * of key & value pairs from input page to newly created page
 */
BPlusTreeInternalPage *BPlusTree::Split(InternalPage *node, Txn *transaction) {
  page_id_t page_id;
  Page *page = buffer_pool_manager_->NewPage(page_id);
  ASSERT(page != nullptr, "Failed to allocate internal split page.");
  auto new_node = reinterpret_cast<InternalPage *>(page->GetData());
  new_node->Init(page_id, node->GetParentPageId(), processor_.GetKeySize(), internal_max_size_);
  node->MoveHalfTo(new_node, buffer_pool_manager_);
  return new_node;
}

BPlusTreeLeafPage *BPlusTree::Split(LeafPage *node, Txn *transaction) {
  page_id_t page_id;
  Page *page = buffer_pool_manager_->NewPage(page_id);
  ASSERT(page != nullptr, "Failed to allocate leaf split page.");
  auto new_node = reinterpret_cast<LeafPage *>(page->GetData());
  new_node->Init(page_id, node->GetParentPageId(), processor_.GetKeySize(), leaf_max_size_);
  node->MoveHalfTo(new_node);
  return new_node;
}

/*
 * Insert key & value pair into internal page after split
 * @param   old_node      input page from split() method
 * @param   key
 * @param   new_node      returned page from split() method
 * User needs to first find the parent page of old_node, parent node must be
 * adjusted to take info of new_node into account. Remember to deal with split
 * recursively if necessary.
 */
void BPlusTree::InsertIntoParent(BPlusTreePage *old_node, GenericKey *key, BPlusTreePage *new_node, Txn *transaction) {
  if (old_node->IsRootPage()) {
    page_id_t root_page_id;
    Page *root_page = buffer_pool_manager_->NewPage(root_page_id);
    ASSERT(root_page != nullptr, "Failed to allocate new internal root page.");
    auto root = reinterpret_cast<InternalPage *>(root_page->GetData());
    root->Init(root_page_id, INVALID_PAGE_ID, processor_.GetKeySize(), internal_max_size_);
    root->PopulateNewRoot(old_node->GetPageId(), key, new_node->GetPageId());
    old_node->SetParentPageId(root_page_id);
    new_node->SetParentPageId(root_page_id);
    root_page_id_ = root_page_id;
    buffer_pool_manager_->UnpinPage(root_page_id, true);
    UpdateRootPageId(0);
    return;
  }

  Page *parent_page = buffer_pool_manager_->FetchPage(old_node->GetParentPageId());
  ASSERT(parent_page != nullptr, "Failed to fetch parent page.");
  auto parent = reinterpret_cast<InternalPage *>(parent_page->GetData());
  parent->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());
  new_node->SetParentPageId(parent->GetPageId());
  if (parent->GetSize() <= parent->GetMaxSize()) {
    buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
    return;
  }
  InternalPage *new_internal = Split(parent, transaction);
  InsertIntoParent(parent, new_internal->KeyAt(0), new_internal, transaction);
  buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(new_internal->GetPageId(), true);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immediately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
void BPlusTree::Remove(const GenericKey *key, Txn *transaction) {
  if (IsEmpty()) {
    return;
  }
  Page *leaf_page = FindLeafPage(key);
  if (leaf_page == nullptr) {
    return;
  }
  auto leaf = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  int old_size = leaf->GetSize();
  leaf->RemoveAndDeleteRecord(key, processor_);
  if (leaf->GetSize() == old_size) {
    buffer_pool_manager_->UnpinPage(leaf->GetPageId(), false);
    return;
  }
  CoalesceOrRedistribute(leaf, transaction);
}

/* todo
 * User needs to first find the sibling of input page. If sibling's size + input
 * page's size > page's max size, then redistribute. Otherwise, merge.
 * Using template N to represent either internal page or leaf page.
 * @return: true means target leaf page should be deleted, false means no
 * deletion happens
 */
template <typename N>
bool BPlusTree::CoalesceOrRedistribute(N *&node, Txn *transaction) {
  if (node->IsRootPage()) {
    page_id_t node_page_id = node->GetPageId();
    bool should_delete = AdjustRoot(node);
    buffer_pool_manager_->UnpinPage(node_page_id, true);
    if (should_delete) {
      buffer_pool_manager_->DeletePage(node_page_id);
    }
    return should_delete;
  }
  if (node->GetSize() >= node->GetMinSize()) {
    buffer_pool_manager_->UnpinPage(node->GetPageId(), true);
    return false;
  }

  Page *parent_page = buffer_pool_manager_->FetchPage(node->GetParentPageId());
  ASSERT(parent_page != nullptr, "Failed to fetch parent page.");
  auto parent = reinterpret_cast<InternalPage *>(parent_page->GetData());
  int index = parent->ValueIndex(node->GetPageId());
  ASSERT(index != -1, "Failed to find child in parent.");
  int neighbor_index = index == 0 ? 1 : index - 1;
  Page *neighbor_page = buffer_pool_manager_->FetchPage(parent->ValueAt(neighbor_index));
  ASSERT(neighbor_page != nullptr, "Failed to fetch neighbor page.");
  auto neighbor = reinterpret_cast<N *>(neighbor_page->GetData());
  if (neighbor->GetSize() + node->GetSize() > node->GetMaxSize()) {
    Redistribute(neighbor, node, index);
    buffer_pool_manager_->UnpinPage(neighbor->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
    return false;
  }
  return Coalesce(neighbor, node, parent, index, transaction);
}

/*
 * Move all the key & value pairs from one page to its sibling page, and notify
 * buffer pool manager to delete this page. Parent page must be adjusted to
 * take info of deletion into account. Remember to deal with coalesce or
 * redistribute recursively if necessary.
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 * @param   parent             parent page of input "node"
 * @return  true means parent node should be deleted, false means no deletion happened
 */
bool BPlusTree::Coalesce(LeafPage *&neighbor_node, LeafPage *&node, InternalPage *&parent, int index,
                         Txn *transaction) {
  LeafPage *left = nullptr;
  LeafPage *right = nullptr;
  int parent_remove_index = 0;
  if (index == 0) {
    left = node;
    right = neighbor_node;
    parent_remove_index = 1;
  } else {
    left = neighbor_node;
    right = node;
    parent_remove_index = index;
  }
  right->MoveAllTo(left);
  page_id_t left_page_id = left->GetPageId();
  page_id_t right_page_id = right->GetPageId();
  parent->Remove(parent_remove_index);
  buffer_pool_manager_->UnpinPage(left_page_id, true);
  buffer_pool_manager_->UnpinPage(right_page_id, true);
  buffer_pool_manager_->DeletePage(right_page_id);
  if (parent->IsRootPage() || parent->GetSize() < parent->GetMinSize()) {
    return CoalesceOrRedistribute(parent, transaction);
  }
  buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
  return false;
}

bool BPlusTree::Coalesce(InternalPage *&neighbor_node, InternalPage *&node, InternalPage *&parent, int index,
                         Txn *transaction) {
  InternalPage *left = nullptr;
  InternalPage *right = nullptr;
  int parent_remove_index = 0;
  if (index == 0) {
    left = node;
    right = neighbor_node;
    parent_remove_index = 1;
  } else {
    left = neighbor_node;
    right = node;
    parent_remove_index = index;
  }
  right->MoveAllTo(left, parent->KeyAt(parent_remove_index), buffer_pool_manager_);
  page_id_t left_page_id = left->GetPageId();
  page_id_t right_page_id = right->GetPageId();
  parent->Remove(parent_remove_index);
  buffer_pool_manager_->UnpinPage(left_page_id, true);
  buffer_pool_manager_->UnpinPage(right_page_id, true);
  buffer_pool_manager_->DeletePage(right_page_id);
  if (parent->IsRootPage() || parent->GetSize() < parent->GetMinSize()) {
    return CoalesceOrRedistribute(parent, transaction);
  }
  buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
  return false;
}

/*
 * Redistribute key & value pairs from one page to its sibling page. If index ==
 * 0, move sibling page's first key & value pair into end of input "node",
 * otherwise move sibling page's last key & value pair into head of input
 * "node".
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 */
void BPlusTree::Redistribute(LeafPage *neighbor_node, LeafPage *node, int index) {
  Page *parent_page = buffer_pool_manager_->FetchPage(node->GetParentPageId());
  ASSERT(parent_page != nullptr, "Failed to fetch parent page.");
  auto parent = reinterpret_cast<InternalPage *>(parent_page->GetData());
  if (index == 0) {
    neighbor_node->MoveFirstToEndOf(node);
    parent->SetKeyAt(1, neighbor_node->KeyAt(0));
  } else {
    neighbor_node->MoveLastToFrontOf(node);
    parent->SetKeyAt(index, node->KeyAt(0));
  }
  buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
}
void BPlusTree::Redistribute(InternalPage *neighbor_node, InternalPage *node, int index) {
  Page *parent_page = buffer_pool_manager_->FetchPage(node->GetParentPageId());
  ASSERT(parent_page != nullptr, "Failed to fetch parent page.");
  auto parent = reinterpret_cast<InternalPage *>(parent_page->GetData());
  if (index == 0) {
    neighbor_node->MoveFirstToEndOf(node, parent->KeyAt(1), buffer_pool_manager_);
    parent->SetKeyAt(1, neighbor_node->KeyAt(0));
  } else {
    GenericKey *new_parent_key = processor_.InitKey();
    memcpy(new_parent_key, neighbor_node->KeyAt(neighbor_node->GetSize() - 1), processor_.GetKeySize());
    neighbor_node->MoveLastToFrontOf(node, parent->KeyAt(index), buffer_pool_manager_);
    parent->SetKeyAt(index, new_parent_key);
    free(new_parent_key);
  }
  buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
}
/*
 * Update root page if necessary
 * NOTE: size of root page can be less than min size and this method is only
 * called within coalesceOrRedistribute() method
 * case 1: when you delete the last element in root page, but root page still
 * has one last child
 * case 2: when you delete the last element in whole b+ tree
 * @return : true means root page should be deleted, false means no deletion
 * happened
 */
bool BPlusTree::AdjustRoot(BPlusTreePage *old_root_node) {
  if (!old_root_node->IsLeafPage() && old_root_node->GetSize() == 1) {
    auto root = reinterpret_cast<InternalPage *>(old_root_node);
    page_id_t new_root_page_id = root->RemoveAndReturnOnlyChild();
    Page *new_root_page = buffer_pool_manager_->FetchPage(new_root_page_id);
    ASSERT(new_root_page != nullptr, "Failed to fetch new root page.");
    auto new_root = reinterpret_cast<BPlusTreePage *>(new_root_page->GetData());
    new_root->SetParentPageId(INVALID_PAGE_ID);
    root_page_id_ = new_root_page_id;
    buffer_pool_manager_->UnpinPage(new_root_page_id, true);
    UpdateRootPageId(0);
    return true;
  }
  if (old_root_node->IsLeafPage() && old_root_node->GetSize() == 0) {
    root_page_id_ = INVALID_PAGE_ID;
    Page *roots_page = buffer_pool_manager_->FetchPage(INDEX_ROOTS_PAGE_ID);
    ASSERT(roots_page != nullptr, "Failed to fetch index roots page.");
    reinterpret_cast<IndexRootsPage *>(roots_page->GetData())->Delete(index_id_);
    buffer_pool_manager_->UnpinPage(INDEX_ROOTS_PAGE_ID, true);
    return true;
  }
  return false;
}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the left most leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
IndexIterator BPlusTree::Begin() {
  if (IsEmpty()) {
    return End();
  }
  Page *leaf_page = FindLeafPage(nullptr, INVALID_PAGE_ID, true);
  while (leaf_page != nullptr) {
    auto leaf = reinterpret_cast<LeafPage *>(leaf_page->GetData());
    if (leaf->GetSize() > 0) {
      page_id_t page_id = leaf->GetPageId();
      buffer_pool_manager_->UnpinPage(page_id, false);
      return IndexIterator(page_id, buffer_pool_manager_, 0);
    }
    page_id_t next_page_id = leaf->GetNextPageId();
    buffer_pool_manager_->UnpinPage(leaf->GetPageId(), false);
    if (next_page_id == INVALID_PAGE_ID) {
      break;
    }
    leaf_page = buffer_pool_manager_->FetchPage(next_page_id);
  }
  return End();
}

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
IndexIterator BPlusTree::Begin(const GenericKey *key) {
  if (IsEmpty()) {
    return End();
  }
  Page *leaf_page = FindLeafPage(key);
  if (leaf_page == nullptr) {
    return End();
  }
  auto leaf = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  int index = leaf->KeyIndex(key, processor_);
  page_id_t page_id = leaf->GetPageId();
  while (index >= leaf->GetSize()) {
    page_id_t next_page_id = leaf->GetNextPageId();
    buffer_pool_manager_->UnpinPage(page_id, false);
    if (next_page_id == INVALID_PAGE_ID) {
      return End();
    }
    Page *next_page = buffer_pool_manager_->FetchPage(next_page_id);
    leaf = reinterpret_cast<LeafPage *>(next_page->GetData());
    page_id = leaf->GetPageId();
    index = 0;
  }
  buffer_pool_manager_->UnpinPage(page_id, false);
  return IndexIterator(page_id, buffer_pool_manager_, index);
}

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
IndexIterator BPlusTree::End() {
  return IndexIterator();
}

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/*
 * Find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 * Note: the leaf page is pinned, you need to unpin it after use.
 */
Page *BPlusTree::FindLeafPage(const GenericKey *key, page_id_t page_id, bool leftMost) {
  if (IsEmpty()) {
    return nullptr;
  }
  page_id_t current_page_id = page_id == INVALID_PAGE_ID ? root_page_id_ : page_id;
  Page *page = buffer_pool_manager_->FetchPage(current_page_id);
  while (page != nullptr) {
    auto node = reinterpret_cast<BPlusTreePage *>(page->GetData());
    if (node->IsLeafPage()) {
      return page;
    }
    auto internal = reinterpret_cast<InternalPage *>(node);
    page_id_t next_page_id = leftMost ? internal->ValueAt(0) : internal->Lookup(key, processor_);
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
    page = buffer_pool_manager_->FetchPage(next_page_id);
  }
  return nullptr;
}

/*
 * Update/Insert root page id in header page(where page_id = INDEX_ROOTS_PAGE_ID,
 * header_page isdefined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      default value is false. When set to true,
 * insert a record <index_name, current_page_id> into header page instead of
 * updating it.
 */
void BPlusTree::UpdateRootPageId(int insert_record) {
  Page *roots_page = buffer_pool_manager_->FetchPage(INDEX_ROOTS_PAGE_ID);
  ASSERT(roots_page != nullptr, "Failed to fetch index roots page.");
  auto roots = reinterpret_cast<IndexRootsPage *>(roots_page->GetData());
  if (insert_record != 0) {
    if (!roots->Insert(index_id_, root_page_id_)) {
      roots->Update(index_id_, root_page_id_);
    }
  } else {
    if (!roots->Update(index_id_, root_page_id_)) {
      roots->Insert(index_id_, root_page_id_);
    }
  }
  buffer_pool_manager_->UnpinPage(INDEX_ROOTS_PAGE_ID, true);
}

/**
 * This method is used for debug only, You don't need to modify
 */
void BPlusTree::ToGraph(BPlusTreePage *page, BufferPoolManager *bpm, std::ofstream &out, Schema *schema) const {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    // Print node name
    out << leaf_prefix << leaf->GetPageId();
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << leaf->GetPageId()
        << ",Parent=" << leaf->GetParentPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      Row ans;
      processor_.DeserializeToKey(leaf->KeyAt(i), ans, schema);
      out << "<TD>" << ans.GetField(0)->toString() << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << leaf->GetPageId() << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << leaf->GetPageId() << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }

    // Print parent links if there is a parent
    if (leaf->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << leaf->GetParentPageId() << ":p" << leaf->GetPageId() << " -> " << leaf_prefix
          << leaf->GetPageId() << ";\n";
    }
  } else {
    auto *inner = reinterpret_cast<InternalPage *>(page);
    // Print node name
    out << internal_prefix << inner->GetPageId();
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << inner->GetPageId()
        << ",Parent=" << inner->GetParentPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        Row ans;
        processor_.DeserializeToKey(inner->KeyAt(i), ans, schema);
        out << ans.GetField(0)->toString();
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Parent link
    if (inner->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << inner->GetParentPageId() << ":p" << inner->GetPageId() << " -> " << internal_prefix
          << inner->GetPageId() << ";\n";
    }
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i))->GetData());
      ToGraph(child_page, bpm, out, schema);
      if (i > 0) {
        auto sibling_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i - 1))->GetData());
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_page->GetPageId() << " " << internal_prefix
              << child_page->GetPageId() << "};\n";
        }
        bpm->UnpinPage(sibling_page->GetPageId(), false);
      }
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

/**
 * This function is for debug only, you don't need to modify
 */
void BPlusTree::ToString(BPlusTreePage *page, BufferPoolManager *bpm) const {
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    std::cout << "Leaf Page: " << leaf->GetPageId() << " parent: " << leaf->GetParentPageId()
              << " next: " << leaf->GetNextPageId() << std::endl;
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  } else {
    auto *internal = reinterpret_cast<InternalPage *>(page);
    std::cout << "Internal Page: " << internal->GetPageId() << " parent: " << internal->GetParentPageId() << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(internal->ValueAt(i))->GetData()), bpm);
      bpm->UnpinPage(internal->ValueAt(i), false);
    }
  }
}

bool BPlusTree::Check() {
  bool all_unpinned = buffer_pool_manager_->CheckAllUnpinned();
  if (!all_unpinned) {
    LOG(ERROR) << "problem in page unpin" << endl;
  }
  return all_unpinned;
}
