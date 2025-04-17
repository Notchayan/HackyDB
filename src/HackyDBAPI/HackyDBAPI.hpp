


// Include necessary headers for BufferPoolManager, CatalogManager, and SQL statements
#include "../BufferPoolManager/BufferPoolManager.hpp"
#include "../CatalogManager/CatalogManager.hpp"
#include "../SQL_Statements/sql_statement.hpp"

#include <string>

// MiniDBAPI class serves as the main interface for interacting with the MiniDB database system
class MiniDBAPI {
private:
    std::string path_;          // Path to the database files
    CatalogManager *cm_;        // Pointer to the CatalogManager for managing metadata
    BufferManager *hdl_;        // Pointer to the BufferManager for managing memory buffers
    std::string curr_db_;       // Name of the currently selected database

public:
    // Constructor to initialize the MiniDBAPI with a given path
    MiniDBAPI(std::string p);

    // Destructor to clean up resources
    ~MiniDBAPI();

    // Quit the database system
    void Quit();

    // Display help information
    void Help();

    // Create a new database
    void CreateDatabase(SQLCreateDatabase &st);

    // Show the list of all databases
    void ShowDatabases();

    // Drop an existing database
    void DropDatabase(SQLDropDatabase &st);

    // Drop an existing table
    void DropTable(SQLDropTable &st);

    // Drop an existing index
    void DropIndex(SQLDropIndex &st);

    // Select a database to use
    void Use(SQLUse &st);

    // Create a new table
    void CreateTable(SQLCreateTable &st);

    // Show the list of all tables in the current database
    void ShowTables();

    // Insert data into a table
    void Insert(SQLInsert &st);

    // Execute a SELECT query
    void Select(SQLSelect &st);

    // Create a new index
    void CreateIndex(SQLCreateIndex &st);

    // Delete data from a table
    void Delete(SQLDelete &st);

    // Update data in a table
    void Update(SQLUpdate &st);
};
