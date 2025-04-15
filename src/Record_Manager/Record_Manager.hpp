#ifndef MINIDB_RECORD_MANAGER_H_
#define MINIDB_RECORD_MANAGER_H_

#include <string>
#include <vector>

#include "BlockInfo/BlockInfo.hpp"
#include "BufferManager/BufferManager.hpp"
#include "CatalogManager/CatalogManager.hpp"
#include "Exceptions/Exceptions.hpp"
#include "SQLStatement/SQLStatement.hpp"

class  RecordManager {
    private:
        BufferManager *hdl_;
        CatalogManager *cm_;
        std::string db_name_;

    public:

    RecordManager(CatalogManager *cm, BufferManager *hdl, std::string db) : cm_(cm), hdl_(hdl), db_name_(db) {}
    ~RecordManager() {}

    // Defining CRUD operations
    void Insert(SQLInsert &st);
    void Select(SQLSelect &st);
    void Delete(SQLDelete &st);
    void Update(SQLUpdate &st);
};

#endif