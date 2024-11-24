#include <iostream>

#include "sqlight.hpp"

int main()
{

    using sqlight::db;
    using sqlight::query;

    auto database = db{ "file:memdb1?mode=memory&cache=shared" };
    auto db2 = std::make_shared<db>("file:memdb1?mode=memory&cache=shared");

    database.execute("PRAGMA compile_options;");

    database.execute("CREATE TABLE IF NOT EXISTS test(id INTEGER NOT NULL, name TEXT NOT NULL);");

    database.execute("INSERT INTO test VALUES (1, 'abc');");
    database.execute("INSERT INTO test VALUES (2, 'def');");
    database.execute("INSERT INTO test VALUES (?, 'ghi');", 3);

    auto transaction = db2->transaction();
    const auto before = transaction.execute<int, std::string>("SELECT * FROM test;");
    transaction.execute("INSERT INTO test VALUES (?, 'jkl');", 4);
    const auto after = transaction.execute<int, std::string>("SELECT * FROM test;");

    const auto results_ = database.execute<int, std::string>("SELECT * FROM test;");
    transaction.commit();

    const auto sql_query = query{ "SELECT * FROM test WHERE id != ?;", 2 };
    const auto results = database.execute<int, std::string>(sql_query);
    const auto results_all = database.execute<int, std::string>("SELECT * FROM test;");
    const auto results2 = database.execute<int, std::string>(sql_query);
}