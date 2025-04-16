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
  

  BPlusTreeNode *BPlusTree::GetNode(int num) {
    BPlusTreeNode *pnode = new BPlusTreeNode(false, this, num);
    return pnode;
  }


void BPlusTree::Print() {
    printf("*****************************************************\n");
    printf("KeyCount: %d, NodeCount: %d, Level: %d, Root: %d \n",
           idx_->key_count(), idx_->node_count(), idx_->level(), idx_->root());
  
    if (idx_->root() != -1) {
      PrintNode(idx_->root());
    }
  }
  
  void BPlusTree::PrintNode(int num) {
    BPlusTreeNode *pnode = GetNode(num);
  
    pnode->Print();
    if (!pnode->GetIsLeaf()) {
  
      for (int i = 0; i <= pnode->GetCount(); i++) {
        PrintNode(pnode->GetValues(i));
      }
    }
  }
  
int BPlusTree::GetVal(TKey key) {
    int ret = -1;
    FindNodeParam fnp = Search(idx_->root(), key);
    if (fnp.flag) {
      ret = fnp.pnode->GetValues(fnp.index);
    }
    return ret;
  }
  


bool BPlusTree::AdjustAfterRemove(int node) {
    BPlusTreeNode *pnode = GetNode(node);
    if (pnode->GetCount() >= idx_->rank()) {
      return true;
    }
  
    if (pnode->IsRoot()) {
      if (pnode->GetCount() == 0) {
        if (!pnode->GetIsLeaf()) {
          idx_->set_root(pnode->GetValues(0));
          GetNode(pnode->GetValues(0))->SetParent(-1);
        } else {
          idx_->set_root(-1);
          idx_->set_leaf_head(-1);
        }
        delete pnode;
        idx_->DecreaseNodeCount();
        idx_->DecreaseLevel();
      }
      return true;
    }
  
    BPlusTreeNode *pbrother;
    BPlusTreeNode *pparent;
    int pos;
  
    pparent = GetNode(pnode->GetParent());
    pparent->Search(pnode->GetKeys(0), pos);
  
    if (pos == pparent->GetCount()) {
      pbrother = GetNode(pparent->GetValues(pos - 1));
  
      if (pbrother->GetCount() > idx_->rank()) {
  
        if (pnode->GetIsLeaf()) {
  
          for (int i = pnode->GetCount(); i > 0; i--) {
            pnode->SetKeys(i, pnode->GetKeys(i - 1));
            pnode->SetValues(i, pnode->GetValues(i - 1));
          }
  
          pnode->SetKeys(0, pbrother->GetKeys(pbrother->GetCount() - 1));
          pnode->SetValues(0, pbrother->GetValues(pbrother->GetCount() - 1));
  
          pnode->SetCount(pnode->GetCount() + 1);
  
          pbrother->SetCount(pbrother->GetCount() - 1);
  
          pparent->SetKeys(pos - 1, pbrother->GetKeys(pbrother->GetCount() - 1));
  
          return true;
        } else {
  
          for (int i = pnode->GetCount(); i > 0; i--) {
            pnode->SetKeys(i, pnode->GetKeys(i - 1));
          }
          for (int i = pnode->GetCount() + 1; i > 0; i--) {
            pnode->SetValues(i, pnode->GetValues(i - 1));
          }
  
          pnode->SetKeys(0, pparent->GetKeys(pos - 1));
          pparent->SetKeys(pos - 1, pbrother->GetKeys(pbrother->GetCount() - 1));
  
          pnode->SetValues(0, pbrother->GetValues(pbrother->GetCount()));
          pnode->SetCount(pnode->GetCount() + 1);
  
          if (pbrother->GetValues(pbrother->GetCount()) >= 0) {
  
            GetNode(pbrother->GetValues(pbrother->GetCount()))
                ->SetParent(pnode->block_num());
            pbrother->SetValues(pbrother->GetCount(), -1);
          }
          pbrother->SetCount(pbrother->GetCount() - 1);
          return true;
        }
      } else {
  
        if (pnode->GetIsLeaf()) {
          pparent->RemoveAt(pos - 1);
          pparent->SetValues(pos - 1, pbrother->block_num());
  
          for (int i = 0; i < pnode->GetCount(); i++) {
            pbrother->SetKeys(pbrother->GetCount() + i, pnode->GetKeys(i));
            pbrother->SetValues(pbrother->GetCount() + i, pnode->GetValues(i));
            pnode->SetValues(i, -1);
          }
  
          pbrother->SetCount(pbrother->GetCount() + pnode->GetCount());
          pbrother->SetNextLeaf(pnode->GetNextLeaf());
          delete pnode;
          idx_->DecreaseNodeCount();
  
          return AdjustAfterRemove(pparent->block_num());
        } else {
          pbrother->SetKeys(pbrother->GetCount(), pparent->GetKeys(pos - 1));
          pbrother->SetCount(pbrother->GetCount() + 1);
          pparent->RemoveAt(pos - 1);
          pparent->SetValues(pos - 1, pbrother->block_num());
          for (int i = 0; i < pnode->GetCount(); i++) {
            pbrother->SetKeys(pbrother->GetCount() + i, pnode->GetKeys(i));
          }
  
          for (int i = 0; i <= pnode->GetCount(); i++) {
            pbrother->SetValues(pbrother->GetCount() + i, pnode->GetValues(i));
            GetNode(pnode->GetValues(i))->SetParent(pbrother->block_num());
          }
  
          pbrother->SetCount(2 * idx_->rank());
  
          delete pnode;
          idx_->DecreaseNodeCount();
  
          return AdjustAfterRemove(pparent->block_num());
        }
      }
  
    } else {
      pbrother = GetNode(pparent->GetValues(pos + 1));
  
      if (pbrother->GetCount() > idx_->rank()) {
  
        if (pnode->GetIsLeaf()) {
          pparent->SetKeys(pos, pbrother->GetKeys(0));
          pnode->SetKeys(pnode->GetCount(), pbrother->GetKeys(0));
          pnode->SetValues(pnode->GetCount(), pbrother->GetValues(0));
          pbrother->SetValues(0, -1);
          pnode->SetCount(pnode->GetCount() + 1);
  
          pbrother->RemoveAt(0);
  
          return true;
        } else {
  
          pnode->SetKeys(pnode->GetCount(), pparent->GetKeys(pos));
          pnode->SetValues(pnode->GetCount() + 1, pbrother->GetValues(0));
          pnode->SetCount(pnode->GetCount() + 1);
          pparent->SetKeys(pos, pbrother->GetKeys(0));
          GetNode(pbrother->GetValues(0))->SetParent(pnode->block_num());
  
          pbrother->RemoveAt(0);
          return true;
        }
      } else {
  
        if (pnode->GetIsLeaf()) {
  
          for (int i = 0; i < idx_->rank(); i++) {
  
            pnode->SetKeys(pnode->GetCount() + i, pbrother->GetKeys(i));
            pnode->SetValues(pnode->GetCount() + i, pbrother->GetValues(i));
            pbrother->SetValues(i, -1);
          }
  
          pnode->SetCount(pnode->GetCount() + idx_->rank());
          delete pbrother;
          idx_->DecreaseNodeCount();
  
          pparent->RemoveAt(pos);
          pparent->SetValues(pos, pnode->block_num());
          return AdjustAfterRemove(pparent->block_num());
        } else {
  
          pnode->SetKeys(pnode->GetCount(), pparent->GetKeys(pos));
  
          pparent->RemoveAt(pos);
  
          pparent->SetValues(pos, pnode->block_num());
  
          pnode->SetCount(pnode->GetCount() + 1);
          for (int i = 0; i < idx_->rank(); i++) {
            pnode->SetKeys(pnode->GetCount() + i, pbrother->GetKeys(i));
          }
  
          for (int i = 0; i <= idx_->rank(); i++) {
            pnode->SetValues(pnode->GetCount() + i, pbrother->GetValues(i));
            GetNode(pbrother->GetValues(i))->SetParent(pnode->block_num());
          }
  
          pnode->SetCount(pnode->GetCount() + idx_->rank());
          delete pbrother;
          idx_->DecreaseNodeCount();
  
          return AdjustAfterRemove(pparent->block_num());
        }
      }
    }
  }


//=======================BPlusTreeNode=======================//

BPlusTreeNode::BPlusTreeNode(bool isnew, BPlusTree *tree, int blocknum, bool newleaf): tree_(tree) {
    is_leaf_ = newleaf;
    rank_ = (tree_->degree() - 1) / 2;
    block_num_ = blocknum;
    GetBuffer();
    if (isnew) {
        SetParent(-1);
        SetNodeType(newleaf ? 1 : 0);
        SetCount(0);
    }
}

bool BPlusTreeNode::GetIsLeaf() { return GetNodeType() == 1; }

  