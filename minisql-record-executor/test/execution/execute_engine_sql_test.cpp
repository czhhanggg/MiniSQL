#include <cstdio>
#include <fstream>
#include <string>

#include "executor/execute_engine.h"
#include "gtest/gtest.h"

extern "C" {
int yyparse(void);
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(const char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE buffer);
}

namespace {

dberr_t RunSql(ExecuteEngine &engine, const std::string &sql) {
  // 这里直接走 parser + execute，验证整条 SQL 执行链。
  MinisqlParserInit();
  auto buffer = yy_scan_string(sql.c_str());
  yyparse();
  if (MinisqlParserGetError()) {
    yy_delete_buffer(buffer);
    MinisqlParserFinish();
    return DB_FAILED;
  }
  auto result = engine.Execute(MinisqlGetParserRootNode());
  yy_delete_buffer(buffer);
  MinisqlParserFinish();
  return result;
}

}  // namespace

TEST(ExecuteEngineSqlTest, BasicSqlFlowTest) {
  std::remove("./databases/sql_flow_test_db");
  std::remove("simple_index_0.dat");
  std::remove("simple_index_1.dat");
  std::remove("simple_index_2.dat");

  ExecuteEngine engine;

  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "create database sql_flow_test_db;"));
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "use sql_flow_test_db;"));
  ASSERT_EQ(DB_SUCCESS,
            RunSql(engine, "create table t1(id int, name char(16) unique, score float, primary key(id));"));
  ASSERT_EQ(DB_TABLE_ALREADY_EXIST,
            RunSql(engine, "create table t1(id int, name char(16), score float);"));
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "create index idx_score on t1(score);"));
  ASSERT_EQ(DB_INDEX_ALREADY_EXIST, RunSql(engine, "create index idx_score on t1(score);"));
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "insert into t1 values(1, \"alice\", 95);"));
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "select * from t1 where id = 1;"));
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "update t1 set name = \"bob\" where id = 1;"));
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "delete from t1 where id = 1;"));
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "drop index idx_score;"));
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "drop table t1;"));
  ASSERT_EQ(DB_QUIT, RunSql(engine, "quit;"));
}

TEST(ExecuteEngineSqlTest, ExecFileTest) {
  std::remove("./databases/sql_exec_file_db");
  std::remove("simple_index_0.dat");
  std::remove("simple_index_1.dat");
  std::remove("simple_index_2.dat");

  std::ofstream script("sql_exec_file_test.sql", std::ios::trunc);
  script << "create database sql_exec_file_db;\n";
  script << "use sql_exec_file_db;\n";
  script << "create table t2(id int, name char(8), score float);\n";
  script << "insert into t2 values(7, \"tom\", 88);\n";
  script << "select * from t2 where id = 7;\n";
  script.close();

  ExecuteEngine engine;
  ASSERT_EQ(DB_SUCCESS, RunSql(engine, "execfile \"sql_exec_file_test.sql\";"));

  std::remove("sql_exec_file_test.sql");
}
