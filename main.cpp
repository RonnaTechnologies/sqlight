#include <iostream>

#include "sqlight.hpp"

int main()
{

    using sqlight::db;
    using sqlight::query;

    db database{ ":memory:" };
    database.execute("CREATE TABLE IF NOT EXISTS test(id INTEGER NOT NULL, name TEXT NOT NULL);");

    database.execute("INSERT INTO test VALUES (1, 'abc');");
    database.execute("INSERT INTO test VALUES (2, 'def');");
    database.execute("INSERT INTO test VALUES (?, 'ghi');", 3);

    const auto sql_query = query{ "SELECT * FROM test WHERE id != ?;", 2 };
    const auto results = database.execute<int, std::string>(sql_query);
    const auto results2 = database.execute<int, std::string>(sql_query);
}