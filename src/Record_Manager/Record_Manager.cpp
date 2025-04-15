#include "Record_Manager.hpp"

#include <iomanip>
#include <iostream>

#include "IndexManager/IndexManager.hpp"

using namespace std;

BlockInfo *RecordManager::GetBlockInfo(Table *table, int blockNumber) {
    if (blockNumber == -1) {
        return nullptr;
    }
    BlockInfo *blockInfo = hdl_->GetFileBlock(db_name_, table->tb_name(), 0, blockNumber);
    return blockInfo;
}


void RecordManager::Insert(SQLInsert &st) {
    string tb_name = st.tb_name();
    unsigned long values_size = st.values().size();
  
    Table *tbl = cm_->GetDB(db_name_)->GetTable(tb_name);
  
    if (tbl == NULL) {
      throw TableNotExistException();
    }
  
    int max_count = (4096 - 12) / (tbl->record_length());
  
    vector<TKey> tkey_values;
    int pk_index = -1;
  
    for (int i = 0; i < values_size; ++i) {
      int value_type = st.values()[i].data_type;
      string value = st.values()[i].value;
      int length = tbl->ats()[i].length();
  
      TKey tmp(value_type, length);
      tmp.ReadValue(value.c_str());
      tkey_values.push_back(tmp);
  
      if (tbl->ats()[i].attr_type() == 1) {
        pk_index = i;
      }
    }
  
    if (pk_index != -1) {
  
      if (tbl->GetIndexNum() != 0) {
  
        BPlusTree tree(tbl->GetIndex(0), hdl_, cm_, db_name_);
  
        int value = tree.GetVal(tkey_values[pk_index]);
        if (value != -1) {
          throw PrimaryKeyConflictException();
        }
      } else {
        int block_num = tbl->first_block_num();
        for (int i = 0; i < tbl->block_count(); ++i) {
          BlockInfo *bp = GetBlockInfo(tbl, block_num);
  
          for (int j = 0; j < bp->GetRecordCount(); ++j) {
            vector<TKey> tkey_value = GetRecord(tbl, block_num, j);
  
            if (tkey_value[pk_index] == tkey_values[pk_index]) {
              throw PrimaryKeyConflictException();
            }
          }
  
          block_num = bp->GetNextBlockNum();
        }
      }
    }
  
    char *content;
    int ub = tbl->first_block_num();    // used block
    int frb = tbl->first_rubbish_num(); // first rubbish block
    int lastub;
    int blocknum, offset;
  
    // find the block for use from the useful block
    while (ub != -1) {
      lastub = ub;
      BlockInfo *bp = GetBlockInfo(tbl, ub);
      if (bp->GetRecordCount() == max_count) {
        ub = bp->GetNextBlockNum();
        continue;
      }
      content =
          bp->GetContentAddress() + bp->GetRecordCount() * tbl->record_length();
      for (vector<TKey>::iterator iter = tkey_values.begin();
           iter != tkey_values.end(); ++iter) {
        memcpy(content, iter->key(), iter->length());
        content += iter->length();
      }
      bp->SetRecordCount(1 + bp->GetRecordCount());
  
      blocknum = ub;
      offset = bp->GetRecordCount() - 1;
  
      hdl_->WriteBlock(bp);
  
      // add record to index
      if (tbl->GetIndexNum() != 0) {
        BPlusTree tree(tbl->GetIndex(0), hdl_, cm_, db_name_);
        for (int i = 0; i < tbl->ats().size(); ++i) {
          if (tbl->GetIndex(0)->attr_name() == tbl->GetIndex(i)->attr_name()) {
            tree.Add(tkey_values[i], blocknum, offset);
            break;
          }
        }
      }
  
      hdl_->WriteToDisk();
      cm_->WriteArchiveFile();
  
      return;
    }
  
    if (frb != -1) {
      BlockInfo *bp = GetBlockInfo(tbl, frb);
      content = bp->GetContentAddress();
      for (vector<TKey>::iterator iter = tkey_values.begin();
           iter != tkey_values.end(); ++iter) {
        memcpy(content, iter->key(), iter->length());
        content += iter->length();
      }
      bp->SetRecordCount(1);
  
      BlockInfo *lastubp = GetBlockInfo(tbl, lastub);
      lastubp->SetNextBlockNum(frb);
  
      tbl->set_first_rubbish_num(bp->GetNextBlockNum());
  
      bp->SetPrevBlockNum(lastub);
      bp->SetNextBlockNum(-1);
  
      blocknum = frb;
      offset = 0;
  
      hdl_->WriteBlock(bp);
      hdl_->WriteBlock(lastubp);
  
    } else {
      // initial or add a block
      int next_block = tbl->first_block_num();
      if (tbl->first_block_num() != -1) {
        BlockInfo *upbp = GetBlockInfo(tbl, tbl->first_block_num());
        upbp->SetPrevBlockNum(tbl->block_count());
        hdl_->WriteBlock(upbp);
      }
      tbl->set_first_block_num(tbl->block_count());
      BlockInfo *bp = GetBlockInfo(tbl, tbl->first_block_num());
  
      bp->SetPrevBlockNum(-1);
      bp->SetNextBlockNum(next_block);
      bp->SetRecordCount(1);
  
      content = bp->GetContentAddress();
      for (vector<TKey>::iterator iter = tkey_values.begin();
           iter != tkey_values.end(); ++iter) {
        memcpy(content, iter->key(), iter->length());
        content += iter->length();
      }
  
      blocknum = tbl->block_count();
      offset = 0;
  
      hdl_->WriteBlock(bp);
  
      tbl->IncreaseBlockCount();
    }
  
    // add record to index
    if (tbl->GetIndexNum() != 0) {
      BPlusTree tree(tbl->GetIndex(0), hdl_, cm_, db_name_);
      for (int i = 0; i < tbl->ats().size(); ++i) {
        if (tbl->GetIndex(0)->attr_name() == tbl->GetIndex(i)->name()) {
          tree.Add(tkey_values[i], blocknum, offset);
          break;
        }
      }
    }
    cm_->WriteArchiveFile();
    hdl_->WriteToDisk();
  }


  void RecordManager::Select(SQLSelect &st) {

    Table *tbl = cm_->GetDB(db_name_)->GetTable(st.tb_name());
  
    for (int i = 0; i < tbl->GetAttributeNum(); ++i) {
      cout << setw(9) << left << tbl->ats()[i].attr_name();
    }
    cout << endl;
  
    vector<vector<TKey>> tkey_values;
  
    bool has_index = false;
    int index_idx;
    int where_idx;
  
    if (tbl->GetIndexNum() != 0) {
      for (int i = 0; i < tbl->GetIndexNum(); ++i) {
        Index *idx = tbl->GetIndex(i);
        for (int j = 0; j < st.wheres().size(); ++j) {
          if (idx->attr_name() == st.wheres()[j].key) {
            if (st.wheres()[j].sign_type == SIGN_EQ) {
              has_index = true;
              index_idx = i;
              where_idx = j;
            }
          }
        }
      }
    }
  
    if (!has_index) {
      int block_num = tbl->first_block_num();
      for (int i = 0; i < tbl->block_count(); ++i) {
        BlockInfo *bp = GetBlockInfo(tbl, block_num);
  
        for (int j = 0; j < bp->GetRecordCount(); ++j) {
          vector<TKey> tkey_value = GetRecord(tbl, block_num, j);
  
          bool sats = true;
  
          for (int k = 0; k < st.wheres().size(); ++k) {
            SQLWhere where = st.wheres()[k];
            if (!SatisfyWhere(tbl, tkey_value, where)) {
              sats = false;
            }
          }
          if (sats) {
            tkey_values.push_back(tkey_value);
          }
        }
  
        block_num = bp->GetNextBlockNum();
      }
    } else { // if has index
      BPlusTree tree(tbl->GetIndex(index_idx), hdl_, cm_, db_name_);
  
      // build TKey for search
      int type = tbl->GetIndex(index_idx)->key_type();
      int length = tbl->GetIndex(index_idx)->key_len();
      std::string value = st.wheres()[where_idx].value;
      TKey dest_key(type, length);
      dest_key.ReadValue(value);
  
      int blocknum = tree.GetVal(dest_key);
  
      if (blocknum != -1) {
        int blockoffset = blocknum;
        blocknum = blocknum >> 16;
        blocknum = blocknum & 0xffff;
        blockoffset = blockoffset & 0xffff;
        vector<TKey> tkey_value = GetRecord(tbl, blocknum, blockoffset);
        bool sats = true;
  
        for (int k = 0; k < st.wheres().size(); ++k) {
          SQLWhere where = st.wheres()[k];
          if (!SatisfyWhere(tbl, tkey_value, where)) {
            sats = false;
          }
        }
        if (sats) {
          tkey_values.push_back(tkey_value);
        }
      }
    }
  
    for (int i = 0; i < tkey_values.size(); ++i) {
      for (int j = 0; j < tkey_values[i].size(); ++j) {
        cout << tkey_values[i][j];
      }
      cout << endl;
    }
    if (tbl->GetIndexNum() != 0) {
      BPlusTree tree(tbl->GetIndex(0), hdl_, cm_, db_name_);
      tree.Print();
    }
  }
  