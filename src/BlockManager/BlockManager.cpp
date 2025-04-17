#include "BlockManager.hpp"

#include <fstream>
#include <boost/filesystem.hpp>

using namespace std;

//=======================CatalogManager=======================//

// Constructor: Initializes the CatalogManager with a given path and reads the archive file.
CatalogManager::CatalogManager(std::string p) : path_(p) { ReadArchiveFile(); }

// Destructor: Writes the archive file before destruction.
CatalogManager::~CatalogManager() { WriteArchiveFile(); }

// Reads the catalog archive file from disk if it exists.
void CatalogManager::ReadArchiveFile() {
    std::string file_name = path_ + "catalog";
    boost::filesystem::path file_path(file_name);

    file_path.imbue(std::locale("en_US.UTF-8"));

    if (boost::filesystem::exists(file_path)) {
        std::ifstream ifs;
        ifs.open(file_name.c_str(), std::ios::binary);
        boost::archive::binary_iarchive iar(ifs);
        iar >> (*this); // Deserialize the catalog data.
        ifs.close();
    }
}

// Writes the catalog archive file to disk.
void CatalogManager::WriteArchiveFile() {
    std::string file_name = path_ + "catalog";

    std::ofstream ofs;
    ofs.open(file_name.c_str(), std::ios::binary);
    boost::archive::binary_oarchive oar(ofs);
    oar << (*this); // Serialize the catalog data.
    ofs.close();
}

// Creates a new database with the given name.
void CatalogManager::CreateDatabase(std::string dbname) {
    dbs_.push_back(Database(dbname));
}

// Deletes a database with the given name.
void CatalogManager::DeleteDatabase(std::string dbname) {
    for (unsigned int i = 0; i < dbs_.size(); i++) {
        if (dbs_[i].db_name() == dbname) {
            dbs_.erase(dbs_.begin() + i); // Remove the database from the list.
        }
    }
}

// Retrieves a pointer to a database by its name. Returns NULL if not found.
Database *CatalogManager::GetDB(std::string db_name) {
    for (unsigned int i = 0; i < dbs_.size(); ++i) {
        if (dbs_[i].db_name() == db_name) {
            return &dbs_[i];
        }
    }
    return NULL;
}

//=======================Database=============================//

// Constructor: Initializes a Database with a given name.
Database::Database(std::string dbname) : db_name_(dbname) {}

// Creates a new table in the database based on the given SQLCreateTable statement.
void Database::CreateTable(SQLCreateTable &st) {
    int record_length = 0;
    Table tb;
    for (int i = 0; i < st.attrs().size(); ++i) {
        tb.AddAttribute(st.attrs()[i]); // Add attributes to the table.
        record_length += st.attrs()[i].length(); // Calculate the record length.
    }
    tb.set_tb_name(st.tb_name());
    tb.set_record_length(record_length);
    tbs_.push_back(tb); // Add the table to the database.
}

// Drops a table from the database based on the given SQLDropTable statement.
void Database::DropTable(SQLDropTable &st) {
    vector<Table>::iterator i;
    for (i = tbs_.begin(); i != tbs_.end(); i++) {
        if (i->tb_name() == st.tb_name()) {
            tbs_.erase(i); // Remove the table from the list.
            return;
        }
    }
}

// Drops an index from the database based on the given SQLDropIndex statement.
void Database::DropIndex(SQLDropIndex &st) {
    for (int i = 0; i < tbs_.size(); i++) {
        for (vector<Index>::iterator j = tbs_[i].ids().begin();
                 j < tbs_[i].ids().end(); j++) {
            if (j->name() == st.idx_name()) {
                tbs_[i].ids().erase(j); // Remove the index from the table.
                return;
            }
        }
    }
}

// Retrieves a pointer to a table by its name. Returns NULL if not found.
Table *Database::GetTable(std::string tb_name) {
    for (unsigned int i = 0; i < tbs_.size(); ++i) {
        if (tbs_[i].tb_name() == tb_name) {
            return &tbs_[i];
        }
    }
    return NULL;
}

// Checks if an index with the given name exists in the database.
bool Database::CheckIfIndexExists(std::string index_name) {
    bool exists = false;
    for (unsigned int i = 0; i < tbs_.size(); ++i) {
        for (unsigned int j = 0; j < tbs_[i].ids().size(); ++j) {
            if (tbs_[i].ids()[j].name() == index_name) {
                exists = true;
            }
        }
    }
    return exists;
}

//=======================Table===============================//

// Retrieves a pointer to an attribute by its name. Returns NULL if not found.
Attribute *Table::GetAttribute(std::string name) {
    for (unsigned int i = 0; i < ats_.size(); ++i) {
        if (ats_[i].attr_name() == name) {
            return &ats_[i];
        }
    }
    return NULL;
}

// Retrieves the index of an attribute by its name. Returns -1 if not found.
int Table::GetAttributeIndex(std::string name) {
    for (unsigned int i = 0; i < ats_.size(); ++i) {
        if (ats_[i].attr_name() == name) {
            return i;
        }
    }
    return -1;
}