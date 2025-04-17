#include <string>
#include <vector>

#include "../HackyDBAPI/HackyDBAPI.hpp"
#include "../SQL_Statements/sql_statement.hpp"

// The Interpreter class is responsible for parsing, formatting, and executing SQL statements.
class Interpreter {
private:
    MiniDBAPI *api; // Pointer to the MiniDBAPI instance for database operations.
    std::string sql_statement_; // The raw SQL statement provided by the user.
    std::vector<std::string> sql_vector_; // A vector to store formatted SQL components.
    int sql_type_; // An integer representing the type of SQL statement (e.g., SELECT, INSERT, etc.).

    // Determines the type of the SQL statement (e.g., SELECT, INSERT, DELETE).
    void TellSQLType();

    // Formats the raw SQL statement into a structured format for processing.
    void FormatSQL();

    // Executes the formatted SQL statement using the MiniDBAPI.
    void Run();

public:
    // Constructor to initialize the Interpreter object.
    Interpreter();

    // Destructor to clean up resources used by the Interpreter object.
    ~Interpreter();

    // Executes the given SQL statement by parsing, formatting, and running it.
    void ExecSQL(std::string statement);
};
