#include "HackyDBAPI.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>

#include "../CatalogManager/CatalogManager.hpp"
#include "../Exception/Exception.hpp"
#include "../IndexManager/IndexManager.hpp"
#include "../RecordManager/RecordManager.hpp"

using namespace std;

// Constructor: Initializes the MiniDBAPI with the given path and creates a CatalogManager instance
MiniDBAPI::MiniDBAPI(std::string p) : path_(p) { 
    cm_ = new CatalogManager(p); 
}

// Destructor: Cleans up allocated resources
MiniDBAPI::~MiniDBAPI() {
    delete hdl_; // Handle for BufferManager
    delete cm_;  // CatalogManager instance
}

// Quit: Cleans up resources and prints a quitting message
void MiniDBAPI::Quit() {
    delete hdl_;
    delete cm_;
    std::cout << "Quiting..." << std::endl;
}

// Help: Displays the list of supported SQL commands
void MiniDBAPI::Help() {
    std::cout << "MiniDB 1.0.0" << std::endl;
    std::cout << "Implemented SQL types:" << std::endl;
    std::cout << "#QUIT#" << std::endl;
    std::cout << "#HELP#" << std::endl;
    std::cout << "#EXEC#" << std::endl;
    std::cout << "#CREATE DATABASE#" << std::endl;
    std::cout << "#SHOW DATABASES#" << std::endl;
    std::cout << "#USE#" << std::endl;
    std::cout << "#DROP DATABASE#" << std::endl;
    std::cout << "#CREATE TABLE#" << std::endl;
    std::cout << "#SHOW TABLES#" << std::endl;
    std::cout << "#DROP TABLES#" << std::endl;
    std::cout << "#CREATE INDEX#" << std::endl;
    std::cout << "#DROP INDEX#" << std::endl;
    std::cout << "#SELECT#" << std::endl;
    std::cout << "#INSERT#" << std::endl;
    std::cout << "#DELETE#" << std::endl;
    std::cout << "#UPDATE#" << std::endl;
}

// CreateDatabase: Creates a new database and its associated folder
void MiniDBAPI::CreateDatabase(SQLCreateDatabase &st) {
    std::cout << "Creating database: " << st.db_name() << std::endl;
    std::string folder_name(path_ + st.db_name());
    boost::filesystem::path folder_path(folder_name);

    folder_path.imbue(std::locale("en_US.UTF-8"));

    // Check if the database already exists
    if (cm_->GetDB(st.db_name()) != NULL) {
        throw DatabaseAlreadyExistsException();
    }

    // Remove existing folder if it exists
    if (boost::filesystem::exists(folder_path)) {
        boost::filesystem::remove_all(folder_path);
        std::cout << "Database folder exists and deleted!" << std::endl;
    }

    // Create the database folder
    boost::filesystem::create_directories(folder_path);
    std::cout << "Database folder created!" << std::endl;

    // Update the catalog
    cm_->CreateDatabase(st.db_name());
    std::cout << "Catalog written!" << std::endl;
    cm_->WriteArchiveFile();
}

// ShowDatabases: Displays the list of all databases
void MiniDBAPI::ShowDatabases() {
    std::vector<Database> dbs = cm_->dbs();
    std::cout << "DATABASE LIST:" << std::endl;
    for (unsigned int i = 0; i < dbs.size(); ++i) {
        Database db = dbs[i];
        std::cout << "\t" << db.db_name() << std::endl;
    }
}

// DropDatabase: Deletes a database and its associated folder
void MiniDBAPI::DropDatabase(SQLDropDatabase &st) {
    std::cout << "Dropping database: " << st.db_name() << std::endl;

    bool found = false;

    // Check if the database exists
    std::vector<Database> dbs = cm_->dbs();
    for (unsigned int i = 0; i < dbs.size(); ++i) {
        if (dbs[i].db_name() == st.db_name()) {
            found = true;
        }
    }

    if (found == false) {
        throw DatabaseNotExistException();
    }

    std::string folder_name(path_ + st.db_name());
    boost::filesystem::path folder_path(folder_name);

    folder_path.imbue(std::locale("en_US.UTF-8"));

    // Remove the database folder if it exists
    if (!boost::filesystem::exists(folder_path)) {
        std::cout << "Database folder doesn't exists!" << std::endl;
    } else {
        boost::filesystem::remove_all(folder_path);
        std::cout << "Database folder deleted!" << std::endl;
    }

    // Update the catalog
    cm_->DeleteDatabase(st.db_name());
    std::cout << "Database removed from catalog!" << std::endl;
    cm_->WriteArchiveFile();

    // Reset the current database if it was the one being dropped
    if (st.db_name() == curr_db_) {
        curr_db_ = "";
        delete hdl_;
    }
}

// Use: Switches to a specific database
void MiniDBAPI::Use(SQLUse &st) {
    Database *db = cm_->GetDB(st.db_name());

    if (db == NULL) {
        throw DatabaseNotExistException();
    }

    // Close the current database if one is already in use
    if (curr_db_.length() != 0) {
        std::cout << "Closing the old database: " << curr_db_ << std::endl;
        cm_->WriteArchiveFile();
        delete hdl_;
    }
    curr_db_ = st.db_name();
    hdl_ = new BufferManager(path_);
}

// CreateTable: Creates a new table in the current database
void MiniDBAPI::CreateTable(SQLCreateTable &st) {
    std::cout << "Creating table: " << st.tb_name() << std::endl;
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Database *db = cm_->GetDB(curr_db_);
    if (db == NULL) {
        throw DatabaseNotExistException();
    }

    // Check if the table already exists
    if (db->GetTable(st.tb_name()) != NULL) {
        throw TableAlreadyExistsException();
    }

    // Create the table file
    std::string file_name(path_ + curr_db_ + "/" + st.tb_name() + ".records");
    boost::filesystem::path folder_path(file_name);

    if (boost::filesystem::exists(file_name)) {
        boost::filesystem::remove(file_name);
        std::cout << "Table file already exists and deleted!" << std::endl;
    }

    ofstream ofs(file_name);
    ofs.close();
    std::cout << "Table file created!" << std::endl;

    // Update the catalog
    db->CreateTable(st);
    std::cout << "Catalog written!" << std::endl;
    cm_->WriteArchiveFile();
}

// ShowTables: Displays the list of tables in the current database
void MiniDBAPI::ShowTables() {
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Database *db = cm_->GetDB(curr_db_);
    if (db == NULL) {
        throw DatabaseNotExistException();
    }
    std::cout << "CURRENT DATABASE: " << curr_db_ << std::endl;
    std::cout << "TABLE LIST:" << std::endl;
    for (int i = 0; i < db->tbs().size(); ++i) {
        Table tb = db->tbs()[i];
        std::cout << "\t" << tb.tb_name() << std::endl;
    }
}

// Insert: Inserts a record into a table
void MiniDBAPI::Insert(SQLInsert &st) {
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Database *db = cm_->GetDB(curr_db_);
    if (db == NULL) {
        throw DatabaseNotExistException();
    }

    RecordManager *rm = new RecordManager(cm_, hdl_, curr_db_);
    rm->Insert(st);
    delete rm;
}

// Select: Retrieves records from a table
void MiniDBAPI::Select(SQLSelect &st) {
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Table *tb = cm_->GetDB(curr_db_)->GetTable(st.tb_name());

    if (tb == NULL) {
        throw TableNotExistException();
    }

    RecordManager *rm = new RecordManager(cm_, hdl_, curr_db_);
    rm->Select(st);
    delete rm;
}

// CreateIndex: Creates an index on a table
void MiniDBAPI::CreateIndex(SQLCreateIndex &st) {
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Database *db = cm_->GetDB(curr_db_);

    if (db->GetTable(st.tb_name()) == NULL) {
        throw TableNotExistException();
    }

    if (db->CheckIfIndexExists(st.index_name())) {
        throw IndexAlreadyExistsException();
    }

    IndexManager *im = new IndexManager(cm_, hdl_, curr_db_);
    im->CreateIndex(st);
    delete im;
}

// DropTable: Deletes a table and its associated files
void MiniDBAPI::DropTable(SQLDropTable &st) {
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Database *db = cm_->GetDB(curr_db_);
    if (db == NULL) {
        throw DatabaseNotExistException();
    }

    Table *tb = cm_->GetDB(curr_db_)->GetTable(st.tb_name());

    if (tb == NULL) {
        throw TableNotExistException();
    }

    // Remove the table file
    std::string file_name(path_ + curr_db_ + "/" + st.tb_name() + ".records");

    if (!boost::filesystem::exists(file_name)) {
        std::cout << "Table file doesn't exist!" << std::endl;
    } else {
        boost::filesystem::remove(file_name);
        std::cout << "Table file removed!" << std::endl;
    }

    // Remove associated index files
    std::cout << "Removing Index files!" << std::endl;
    for (int i = 0; i < tb->GetIndexNum(); ++i) {
        std::string file_name(path_ + curr_db_ + "/" + tb->GetIndex(i)->name() +
                                                    ".index");
        if (!boost::filesystem::exists(file_name)) {
            std::cout << "Index file doesn't exist!" << std::endl;
        } else {
            boost::filesystem::remove(file_name);
            std::cout << "Index file removed!" << std::endl;
        }
    }

    // Update the catalog
    db->DropTable(st);
    std::cout << "Catalog written!" << std::endl;
    cm_->WriteArchiveFile();
}

// DropIndex: Deletes an index file
void MiniDBAPI::DropIndex(SQLDropIndex &st) {
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Database *db = cm_->GetDB(curr_db_);
    if (db == NULL) {
        throw DatabaseNotExistException();
    }

    if (!db->CheckIfIndexExists(st.idx_name())) {
        throw IndexNotExistException();
    }

    // Remove the index file
    std::string file_name(path_ + curr_db_ + "/" + st.idx_name() + ".index");

    if (!boost::filesystem::exists(file_name)) {
        std::cout << "Index file doesn't exist!" << std::endl;
        return;
    }
    boost::filesystem::remove(file_name);
    std::cout << "Index file removed!" << std::endl;

    // Update the catalog
    db->DropIndex(st);
    std::cout << "Catalog written!" << std::endl;
    cm_->WriteArchiveFile();
}

// Delete: Deletes records from a table
void MiniDBAPI::Delete(SQLDelete &st) {
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Database *db = cm_->GetDB(curr_db_);
    if (db == NULL) {
        throw DatabaseNotExistException();
    }

    Table *tb = cm_->GetDB(curr_db_)->GetTable(st.tb_name());

    if (tb == NULL) {
        throw TableNotExistException();
    }

    RecordManager *rm = new RecordManager(cm_, hdl_, curr_db_);
    rm->Delete(st);
    delete rm;
}

// Update: Updates records in a table
void MiniDBAPI::Update(SQLUpdate &st) {
    if (curr_db_.length() == 0) {
        throw NoDatabaseSelectedException();
    }

    Database *db = cm_->GetDB(curr_db_);
    if (db == NULL) {
        throw DatabaseNotExistException();
    }

    Table *tb = cm_->GetDB(curr_db_)->GetTable(st.tb_name());

    if (tb == NULL) {
        throw TableNotExistException();
    }

    RecordManager *rm = new RecordManager(cm_, hdl_, curr_db_);
    rm->Update(st);
    delete rm;
}