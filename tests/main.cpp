#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <sqlight.hpp>

TEST_CASE("Quick check", "[main]")
{
    using sqlight::db;

    auto database = db{ "file:memdb1?mode=memory&cache=shared" };

    const auto [one] = database.execute<int>("SELECT 1;")[0];

    REQUIRE(one == 1);
}
