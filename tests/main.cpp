#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <sqlight.hpp>

#include <memory>

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
                    database->execute(R"(INSERT INTO "test"("id", "name", "value") VALUES(1, 'test_1', 50);)");
                    database->execute(R"(INSERT INTO "test"("id", "name", "value") VALUES(2, 'test_2', 50);)");

                    AND_THEN("The table ca be read")
                    {
                        const auto records = database->execute<int>(R"(SELECT "id" FROM "test";)");

                        AND_THEN("The records are read successfully")
                        {
                            REQUIRE(records.size() == 2U);

                            const auto [first] = records.front();
                            REQUIRE(first == 1);

                            const auto [second] = records.back();
                            REQUIRE(second == 2);
                        }
                    }
                }
            }

            THEN("A transaction can be created")
            {
                auto transaction = database->transaction();
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
