#include "IndexManager.hpp"

#include <fstream>
#include <iostream>

#include "../Commons/Commons.hpp"
#include "../Exceptions/Exceptions.hpp"
#include "../RecordManager/RecordManager.hpp"
#include "../SQLStatement/SQLStatement.hpp"

using namespace std;


//=======================IndexManager=======================//

void IndexManager::CreateIndex(SQLCreateIndex &st) {
    string tb_name = st.tb_name();
  
    Table *tbl = cm_->GetDB(db_name_)->GetTable(tb_name);
  
    if (tbl == NULL) {
      throw TableNotExistException();
    }
  
    if (tbl->GetIndexNum() != 0) {
      throw OneIndexEachTableException();
    }
  
    Attribute *attr = tbl->GetAttribute(st.col_name());
    if (attr->attr_type() != 1) {
      throw IndexMustBeCreatedOnPKException();
    }
  
    string file_name = cm_->path() + db_name_ + "/" + st.index_name() + ".index";
    std::ofstream ofs(file_name.c_str(), std::ios::binary);
    ofs.close();
  
    Index idx(st.index_name(), st.col_name(), attr->data_type(), attr->length(),
              (4 * 1024 - 12) / (4 + attr->length()) / 2 - 1);
  
    tbl->AddIndex(idx);
  
    BPlusTree tree(tbl->GetIndex(0), hdl_, cm_, db_name_);
  
    RecordManager *rm = new RecordManager(cm_, hdl_, db_name_);
  
    int col_idx = tbl->GetAttributeIndex(st.col_name());
  
    int block_num = tbl->first_block_num();
    for (int i = 0; i < tbl->block_count(); ++i) {
      BlockInfo *bp = rm->GetBlockInfo(tbl, block_num);
  
      for (int j = 0; j < bp->GetRecordCount(); ++j) {
        vector<TKey> tkey_value = rm->GetRecord(tbl, block_num, j);
        tree.Add(tkey_value[col_idx], block_num, j);
      }
  
      block_num = bp->GetNextBlockNum();
    }
  
    delete rm;
  
    hdl_->WriteToDisk();
    cm_->WriteArchiveFile();
  
    tree.Print();
  }


void BPlusTree::InitTree() {
    BPlusTreeNode *root_node =
        new BPlusTreeNode(true, this, GetNewBlockNum(), true);
    idx_->set_root(0);
    idx_->set_leaf_head(idx_->root());
    idx_->set_key_count(0);
    idx_->set_node_count(1);
    idx_->set_level(1);
    root_node->SetNextLeaf(-1);
  }
  
  bool BPlusTree::Add(TKey &key, int block_num, int offset) {
    int value = (block_num << 16) | offset;
  
    if (idx_->root() == -1) {
      InitTree();
    }
  
    FindNodeParam fnp = Search(idx_->root(), key);
  
    if (!fnp.flag) {
      fnp.pnode->Add(key, value);
      idx_->IncreaseKeyCount();
  
      if (fnp.pnode->GetCount() == degree_) {
        return AdjustAfterAdd(fnp.pnode->block_num());
      }
  
      return true;
    }
  
    return false;
  }

  bool BPlusTree::AdjustAfterAdd(int node) {
    BPlusTreeNode *pnode = GetNode(node);
    TKey key(idx()->key_type(), idx()->key_len());
    BPlusTreeNode *newnode = pnode->Split(key);
    idx_->IncreaseNodeCount();
    int parent = pnode->GetParent();
  
    if (parent == -1) {
      BPlusTreeNode *newroot = new BPlusTreeNode(true, this, GetNewBlockNum());
      if (newroot == NULL)
        return false;
  
      idx_->IncreaseNodeCount();
      idx_->set_root(newroot->block_num());
  
      newroot->Add(key);
  
      newroot->SetValues(0, pnode->block_num());
      newroot->SetValues(1, newnode->block_num());
  
      pnode->SetParent(idx_->root());
      newnode->SetParent(idx_->root());
      newnode->SetNextLeaf(-1);
      idx_->IncreaseLevel();
      return true;
    } else {
      BPlusTreeNode *parentnode = GetNode(parent);
      int index = parentnode->Add(key);
  
      parentnode->SetValues(index, pnode->block_num());
      parentnode->SetValues(index + 1, newnode->block_num());
  
      if (parentnode->GetCount() == degree_) {
        return AdjustAfterAdd(parentnode->block_num());
      }
      return true;
    }
  }
  


FindNodeParam BPlusTree::Search(int node, TKey &key) {
    FindNodeParam ret;
    int index = 0;
    BPlusTreeNode *pnode = GetNode(node);
    if (pnode->Search(key, index)) {
      if (pnode->GetIsLeaf()) {
        ret.flag = true;
        ret.index = index;
        ret.pnode = pnode;
      } else {
        pnode = GetNode(pnode->GetValues(index));
        while (!pnode->GetIsLeaf()) {
          pnode = GetNode(pnode->GetValues(pnode->GetCount()));
        }
        ret.flag = true;
        ret.index = pnode->GetCount() - 1;
        ret.pnode = pnode;
      }
    } else {
      if (pnode->GetIsLeaf()) {
        ret.flag = false;
        ret.index = index;
        ret.pnode = pnode;
      } else {
        return Search(pnode->GetValues(index), key);
      }
    }
  
    return ret;
  }
  


  FindNodeParam BPlusTree::SearchBranch(int node, TKey &key) {
    FindNodeParam ret;
    int index = 0;
    BPlusTreeNode *pnode = GetNode(node);
  
    if (pnode->GetIsLeaf()) {
      throw BPlusTreeException();
    }
    if (pnode->Search(key, index)) {
      ret.flag = true;
      ret.index = index;
      ret.pnode = pnode;
    } else {
      if (!GetNode(pnode->GetValues(index))->GetIsLeaf()) {
        ret = SearchBranch(pnode->GetValues(index), key);
      } else {
        ret.index = index;
        ret.flag = false;
        ret.pnode = pnode;
      }
    }
    return ret;
  }
  