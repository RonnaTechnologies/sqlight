#include <iostream>

#include "sqlight.hpp"

int main() {

  using sqlight::db;
  using sqlight::query;

  db database{":memory:"};
  database.exec("CREATE TABLE IF NOT EXISTS test(id INTEGER NOT NULL, name "
                "TEXT NOT NULL);");

  database.exec("INSERT INTO test VALUES (1, 'abc');");
  database.exec("INSERT INTO test VALUES (2, 'def');");
  database.exec("INSERT INTO test VALUES (3, 'ghi');");

  const auto sql_query = query{"SELECT * FROM test WHERE id != ?;", 2};
  const auto results = database.execute<int, std::string>(sql_query);
  const auto results2 = database.execute<int, std::string>(sql_query);
}