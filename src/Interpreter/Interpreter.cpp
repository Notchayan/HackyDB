/*
 * Supported SQL Statements:
 * --------------------------
 * 1. quit                     -> Quit the application
 * 2. help                     -> Display help information
 * 3. create database <name>  -> Create a new database
 * 4. create table <name>     -> Create a new table
 * 5. create index <name>     -> Create a new index
 * 6. show databases          -> List all databases
 * 7. show tables             -> List all tables in the current database
 * 8. drop database <name>    -> Drop an existing database
 * 9. drop table <name>       -> Drop an existing table
 * 10. drop index <name>      -> Drop an existing index
 * 11. use <database_name>    -> Select a database for operations
 * 12. insert into ...        -> Insert data into a table
 * 13. exec <filename>        -> Execute SQL statements from a file
 * 14. select ...             -> Query data from a table
 * 15. delete from ...        -> Delete data from a table
 * 16. update ...             -> Update data in a table
 */

 #include "interpreter.hpp"
 #include <fstream>
 #include <iostream>
 
 #include <boost/algorithm/string.hpp>
 #include <boost/filesystem.hpp>
 #include <boost/regex.hpp>
 
 #include "../Exceptions/Exceptions.hpp"
 #include "../HackyDBAPI/HackyDBAPI.hpp"
 
 using namespace std;
 
 // Constructor initializes MiniDBAPI instance with data directory
 Interpreter::Interpreter() : sql_type_(-1) {
   string p = string(getenv("HOME")) + "/MiniDBData/";
   api = new MiniDBAPI(p);
 }
 
 // Destructor to clean up the API object
 Interpreter::~Interpreter() { delete api; }
 
 // Utility function to split string `str` by delimiter `sep`
 vector<string> split(string str, string sep) {
   char *cstr = const_cast<char *>(str.c_str());
   char *current;
   vector<string> arr;
   current = strtok(cstr, sep.c_str());
   while (current != NULL) {
     arr.push_back(current);
     current = strtok(NULL, sep.c_str());
   }
   return arr;
 }
 
 // Preprocess and tokenize the SQL statement
 void Interpreter::FormatSQL() {
   // remove newlines, tabs
   boost::regex reg("[\r\n\t]");
   sql_statement_ = boost::regex_replace(sql_statement_, reg, " ");
 
   // remove ; and chars after ;
   reg = ";.*$";
   sql_statement_ = boost::regex_replace(sql_statement_, reg, "");
 
   // remove leading and trailing spaces
   reg = "(^ +)|( +$)";
   sql_statement_ = boost::regex_replace(sql_statement_, reg, "");
 
   // remove duplicate spaces
   reg = " +";
   sql_statement_ = boost::regex_replace(sql_statement_, reg, " ");
 
   // insert space before/after special symbols
   reg = " ?(\\(|\\)|,|=|(<>)|<|>) ?";
   sql_statement_ = boost::regex_replace(sql_statement_, reg, " $1 ");
   reg = "< *>";
   sql_statement_ = boost::regex_replace(sql_statement_, reg, "<>");
   reg = "< *=";
   sql_statement_ = boost::regex_replace(sql_statement_, reg, "<=");
   reg = "> *=";
   sql_statement_ = boost::regex_replace(sql_statement_, reg, ">=");
 
   // split the final SQL statement
   sql_vector_ = split(sql_statement_, " ");
 }
 
 // Determine the type of SQL statement
 void Interpreter::TellSQLType() {
   if (sql_vector_.size() == 0) {
     sql_type_ = -1;
     cout << "SQL TYPE: #EMPTY#" << endl;
     return;
   }
 
   boost::algorithm::to_lower(sql_vector_[0]);
 
   if (sql_vector_[0] == "quit") {
     cout << "SQL TYPE: #QUIT#" << endl;
     sql_type_ = 10;
   } else if (sql_vector_[0] == "help") {
     cout << "SQL TYPE: #HELP#" << endl;
     sql_type_ = 20;
   } else if (sql_vector_[0] == "create") {
     boost::algorithm::to_lower(sql_vector_[1]);
     if (sql_vector_[1] == "database") {
       cout << "SQL TYPE: #CREATE DATABASE#" << endl;
       sql_type_ = 30;
     } else if (sql_vector_[1] == "table") {
       cout << "SQL TYPE: #CREATE TABLE#" << endl;
       sql_type_ = 31;
     } else if (sql_vector_[1] == "index") {
       cout << "SQL TYPE: #CREATE INDEX#" << endl;
       sql_type_ = 32;
     } else {
       sql_type_ = -1;
     }
   } else if (sql_vector_[0] == "show") {
     boost::algorithm::to_lower(sql_vector_[1]);
     if (sql_vector_[1] == "databases") {
       cout << "SQL TYPE: #SHOW DATABASES#" << endl;
       sql_type_ = 40;
     } else if (sql_vector_[1] == "tables") {
       cout << "SQL TYPE: #SHOW TABLES#" << endl;
       sql_type_ = 41;
     } else {
       sql_type_ = -1;
     }
   } else if (sql_vector_[0] == "drop") {
     boost::algorithm::to_lower(sql_vector_[1]);
     if (sql_vector_[1] == "database") {
       cout << "SQL TYPE: #DROP DATABASE#" << endl;
       sql_type_ = 50;
     } else if (sql_vector_[1] == "table") {
       cout << "SQL TYPE: #DROP TABLE#" << endl;
       sql_type_ = 51;
     } else if (sql_vector_[1] == "index") {
       cout << "SQL TYPE: #DROP INDEX#" << endl;
       sql_type_ = 52;
     } else {
       sql_type_ = -1;
     }
   } else if (sql_vector_[0] == "use") {
     cout << "SQL TYPE: #USE#" << endl;
     sql_type_ = 60;
   } else if (sql_vector_[0] == "insert") {
     cout << "SQL TYPE: #INSERT#" << endl;
     sql_type_ = 70;
   } else if (sql_vector_[0] == "exec") {
     cout << "SQL TYPE: #EXEC#" << endl;
     sql_type_ = 80;
   } else if (sql_vector_[0] == "select") {
     cout << "SQL TYPE: #SELECT#" << endl;
     sql_type_ = 90;
   } else if (sql_vector_[0] == "delete") {
     cout << "SQL TYPE: #DELETE#" << endl;
     sql_type_ = 100;
   } else if (sql_vector_[0] == "update") {
     cout << "SQL TYPE: #UPDATE#" << endl;
     sql_type_ = 110;
   } else {
     sql_type_ = -1;
     cout << "SQL TYPE: #UNKNOWN#" << endl;
   }
 }
 
 // Execute the appropriate API call based on the SQL type
 void Interpreter::Run() {
   try {
     switch (sql_type_) {
     case 10: {
       api->Quit();
       exit(0);
     } break;
     case 20: {
       api->Help();
     } break;
     case 30: {
       SQLCreateDatabase *st = new SQLCreateDatabase(sql_vector_);
       api->CreateDatabase(*st);
       delete st;
     } break;
     case 31: {
       SQLCreateTable *st = new SQLCreateTable(sql_vector_);
       api->CreateTable(*st);
       delete st;
     } break;
     case 32: {
       SQLCreateIndex *st = new SQLCreateIndex(sql_vector_);
       api->CreateIndex(*st);
       delete st;
     } break;
     case 40: {
       api->ShowDatabases();
     } break;
     case 41: {
       api->ShowTables();
     } break;
     case 50: {
       SQLDropDatabase *st = new SQLDropDatabase(sql_vector_);
       api->DropDatabase(*st);
       delete st;
     } break;
     case 51: {
       SQLDropTable *st = new SQLDropTable(sql_vector_);
       api->DropTable(*st);
       delete st;
     } break;
     case 52: {
       SQLDropIndex *st = new SQLDropIndex(sql_vector_);
       api->DropIndex(*st);
       delete st;
     } break;
     case 60: {
       SQLUse *st = new SQLUse(sql_vector_);
       api->Use(*st);
       delete st;
     } break;
     case 70: {
       SQLInsert *st = new SQLInsert(sql_vector_);
       api->Insert(*st);
       delete st;
     } break;
     case 80: {
       SQLExec *st = new SQLExec(sql_vector_);
       string contents;
       ifstream in(st->file_name(), ios::in | ios::binary);
       in.seekg(0, std::ios::end);
       contents.resize(in.tellg());
       in.seekg(0, std::ios::beg);
       in.read(&contents[0], contents.size());
       in.close();
       cout << endl;
       vector<string> sqls = split(contents, ";");
       for (int i = 0; i < sqls.size() - 1; ++i) {
         ExecSQL(sqls[i]);
       }
       delete st;
     } break;
     case 90: {
       SQLSelect *st = new SQLSelect(sql_vector_);
       api->Select(*st);
       delete st;
     } break;
     case 100: {
       SQLDelete *st = new SQLDelete(sql_vector_);
       api->Delete(*st);
       delete st;
     } break;
     case 110: {
       SQLUpdate *st = new SQLUpdate(sql_vector_);
       api->Update(*st);
       delete st;
     } break;
     default:
       break;
     }
   } catch (SyntaxErrorException &e) {
     cerr << "Syntax Error!" << endl;
   } catch (NoDatabaseSelectedException &e) {
     cerr << "No database selected!" << endl;
   } catch (DatabaseNotExistException &e) {
     cerr << "Database doesn't exist!" << endl;
   } catch (DatabaseAlreadyExistsException &e) {
     cerr << "Database already exists!" << endl;
   } catch (TableNotExistException &e) {
     cerr << "Table doesn't exist!" << endl;
   } catch (OneIndexEachTableException &e) {
     cerr << "Each table could only have one index!" << endl;
   } catch (BPlusTreeException &e) {
     cerr << "BPlusTree exception!" << endl;
   } catch (TableAlreadyExistsException &e) {
     cerr << "Table already exists!" << endl;
   } catch (IndexAlreadyExistsException &e) {
     cerr << "Index already exists!" << endl;
   } catch (IndexNotExistException &e) {
     cerr << "Index doesn't exist!" << endl;
   } catch (IndexMustBeCreatedOnPKException &e) {
     cerr << "Index must be created on primary key!" << endl;
   } catch (PrimaryKeyConflictException &e) {
     cerr << "Primary key conflicts!" << endl;
   }
 }
 
 // Top-level method to execute a SQL statement string
 void Interpreter::ExecSQL(string statement) {
   sql_statement_ = statement;
   FormatSQL();       // Clean and tokenize
   TellSQLType();     // Determine the SQL type
   cout << "SQL STATEMENT: " << sql_statement_ << endl;
   Run();             // Call API function
   cout << endl;
 }
 