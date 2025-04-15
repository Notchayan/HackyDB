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

  