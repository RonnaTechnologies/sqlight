#include <optional>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <sqlight.hpp>

#include <memory>
#include <vector>


using sqlight::db;

SCENARIO("A database can be created", "[main]")
{
    GIVEN("A database file")
    {
        WHEN("The database file cannot be created")
        {
            const auto db_file = "/____";
            THEN("An exception is thrown")
            {
                REQUIRE_THROWS(db{ db_file });
            }
        }
    }

    GIVEN("An in-memory database")
    {
        WHEN("A database is opened")
        {

            const auto db_file = "file:memdb1?mode=memory&cache=shared";
            auto database = db{ db_file };

            THEN("A transaction cannot be created if the database is not accessed through a shared_ptr")
            {
                REQUIRE_THROWS(database.transaction());
            }
        }
    }

    GIVEN("Another in-memory database")
    {

        WHEN("A database is created via std::make_shared")
        {
            const auto db_file = "file:memdb1?mode=memory&cache=shared";
            auto database = std::make_shared<db>(db_file);

            THEN("A table can be created")
            {
                database->execute(R"(CREATE TABLE "test"("id" INTEGER NOT NULL, "name" TEXT NOT NULL, "value" INTEGER NOT NULL);)");
                AND_THEN("Some data can be inserted into the table")
                {
                    database->execute(R"(INSERT INTO "test"("id", "name", "value") VALUES(1, 'test_1', 50);
                                                    INSERT INTO "test"("id", "name", "value") VALUES(2, 'test_2', 50);)");

                    AND_THEN("The table can be read")
                    {
                        const auto records = database->execute<int>(R"(SELECT "id" FROM "test";)");

                        AND_THEN("The records are read successfully")
                        {
                            REQUIRE(records.size() == 2U);

                            const auto [first] = records.front();
                            REQUIRE(first == 1);

                            const auto [second] = records.back();
                            REQUIRE(second == 2);
                            // AND_THEN("A transaction can be created")
                            // {
                            //     auto transaction = database->transaction();
                            //     const auto result = transaction.execute<int>(R"(SELECT "id" FROM "test";)");
                            //     transaction.commit();
                            // }
                        }
                    }
                }
            }
        }
    }


    // const auto [one] = database.execute<int>("SELECT ?;", 1)[0];

    // auto database2 = std::make_shared<db>(db_file);

    // auto transaction = database2->transaction();
    // transaction.execute("SELECT 1;");
    // transaction.commit();

    // REQUIRE(one == 1);
}

SCENARIO("A blob can be inserted into the DB", "[main]")
{
    GIVEN("A in-memory database")
    {

        AND_GIVEN("A database is created via std::make_shared")
        {
            const auto db_file = "file:memdb2?mode=memory&cache=shared";
            auto database = std::make_shared<db>(db_file);

            AND_GIVEN("A table can be created")
            {
                database->execute(R"(CREATE TABLE "test"("id" INTEGER NOT NULL, "name" TEXT NOT NULL, "data" BLOB);)");
                AND_GIVEN("Some data can be inserted into the table")
                {
                    const auto id = 1;
                    const auto name = std::string{ "test" };
                    const auto data = std::vector<char>{ 1, 2, 3 };

                    database->execute(R"(INSERT INTO "test" VALUES (:id, :name, :data);)", id, name, data);

                    const auto result = database->execute<int, std::string, std::vector<char>>(R"(SELECT * FROM "test";)");

                    REQUIRE(result.size() == 1U);
                    REQUIRE(std::get<0>(result.front()) == id);
                    REQUIRE(std::get<1>(result.front()) == name);
                    REQUIRE(std::get<2>(result.front()) == data);
                }
            }
        }
    }
}

SCENARIO("An optional value can be inserted and retrieved", "[main]")
{
    GIVEN("A in-memory database")
    {

        AND_GIVEN("A database is created via std::make_shared")
        {
            const auto db_file = "file:memdb2?mode=memory&cache=shared";
            auto database = std::make_shared<db>(db_file);

            AND_GIVEN("A table can be created")
            {
                database->execute(R"(CREATE TABLE "test"("id" INTEGER NOT NULL, "name" TEXT NULL);)");
                AND_GIVEN("Some data can be inserted into the table")
                {
                    const auto id = 1;
                    const auto name = std::optional<std::string>{ "test" };

                    database->execute(R"(INSERT INTO "test" VALUES(?, ?))", id, name);


                    const auto id2 = 2;
                    const auto name2 = std::optional<std::string>{ std::nullopt };
                    database->execute(R"(INSERT INTO "test" VALUES(?, ?))", id2, name2);

                    WHEN("Data is fetched")
                    {
                        const auto results =
                        database->execute<int, std::optional<std::string>>(R"(SELECT "id", "name" FROM "test";)");

                        THEN("Optional results are retrieved correctly")
                        {
                            REQUIRE(results.size() == 2U);

                            const auto [first_id, first_name] = results.front();
                            const auto [second_id, second_name] = results.back();

                            REQUIRE(first_name.has_value());
                            REQUIRE(first_name.value() == name);
                            REQUIRE(!second_name.has_value());
                        }
                    }
                }
            }
        }
    }
}
