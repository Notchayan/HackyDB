
#ifndef HackyDb_INTERPRETER_H_
#define HackyDb_INTERPRETER_H_

#include <string>
#include <vector>

#include "../APIs/HackyDB_api.h"
#include "../SQL/sql_statement.h"

class Interpreter {
private:
  HackyDbAPI *api;
  std::string sql_statement_;
  std::vector<std::string> sql_vector_;
  int sql_type_;
  void TellSQLType();
  void FormatSQL();
  void Run();

public:
  Interpreter();
  ~Interpreter();
  void ExecSQL(std::string statement);
};

#endif /* HackyDb_INTERPRETER_H_ */
