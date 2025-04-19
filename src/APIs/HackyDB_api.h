
#ifndef HackyDb_HackyDb_API_H_
#define HackyDb_HackyDb_API_H_

#include <string>

#include "../Core/Buffer/Buffer_manager/buffer_manager.h"
#include "../managers/Catalog_manager/catalog_manager.h"
#include "../SQL/sql_statement.h"

class HackyDbAPI {
private:
  std::string path_;
  CatalogManager *cm_;
  BufferManager *hdl_;
  std::string curr_db_;

public:
  HackyDbAPI(std::string p);
  ~HackyDbAPI();
  void Quit();
  void Help();
  void CreateDatabase(SQLCreateDatabase &st);
  void ShowDatabases();
  void DropDatabase(SQLDropDatabase &st);
  void DropTable(SQLDropTable &st);
  void DropIndex(SQLDropIndex &st);
  void Use(SQLUse &st);
  void CreateTable(SQLCreateTable &st);
  void ShowTables();
  void Insert(SQLInsert &st);
  void Select(SQLSelect &st);
  void CreateIndex(SQLCreateIndex &st);
  void Delete(SQLDelete &st);
  void Update(SQLUpdate &st);
};

#endif /* HackyDb_HackyDb_API_H_ */
