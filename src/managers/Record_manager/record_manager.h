

#ifndef HackyDb_RECORD_MANAGER_H_
#define HackyDb_RECORD_MANAGER_H_

#include <string>
#include <vector>

#include "../../Core/Block/Block_info/block_info.h"
#include "../../Core/Buffer/Buffer_manager/buffer_manager.h"
#include "../Catalog_manager/catalog_manager.h"
#include "../../Includes/exceptions.h"
#include "../../SQL/sql_statement.h"

class RecordManager {
private:
  BufferManager *hdl_;
  CatalogManager *cm_;
  std::string db_name_;

public:
  RecordManager(CatalogManager *cm, BufferManager *hdl, std::string db)
      : cm_(cm), hdl_(hdl), db_name_(db) {}
  ~RecordManager() {}
  void Insert(SQLInsert &st);
  void Select(SQLSelect &st);
  void Delete(SQLDelete &st);
  void Update(SQLUpdate &st);

  BlockInfo *GetBlockInfo(Table *tbl, int block_num);
  std::vector<TKey> GetRecord(Table *tbl, int block_num, int offset);
  void DeleteRecord(Table *tbl, int block_num, int offset);
  void UpdateRecord(Table *tbl, int block_num, int offset,
                    std::vector<int> &indices, std::vector<TKey> &values);

  bool SatisfyWhere(Table *tbl, std::vector<TKey> keys, SQLWhere where);
};

#endif /* HackyDb_RECORD_MANAGER_H_ */
