#include <catch2/catch_test_macros.hpp>

TEST_CASE("smoke: arithmetic still works", "[smoke]") {
    REQUIRE(2 + 2 == 4);
}
