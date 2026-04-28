#include <catch2/catch_test_macros.hpp>

#include "pae/io/GridLoader.hpp"

using pae::io::GridLoader;
using pae::IoError;

TEST_CASE("GridLoader parses a tiny grid", "[io][grid]") {
    const auto text =
        "3 3\n"
        "S.E\n"
        "...\n"
        "...\n";
    auto g = GridLoader::loadFromString(text);
    REQUIRE(g.width()  == 3);
    REQUIRE(g.height() == 3);
    REQUIRE(g.start()  == pae::core::Coord{0, 0});
    REQUIRE(g.end()    == pae::core::Coord{0, 2});
}

TEST_CASE("GridLoader rejects missing Start", "[io][grid]") {
    const auto text =
        "2 2\n"
        "..\n"
        ".E\n";
    REQUIRE_THROWS_AS(GridLoader::loadFromString(text), IoError);
}

TEST_CASE("GridLoader rejects multiple Start", "[io][grid]") {
    const auto text =
        "2 2\n"
        "SS\n"
        ".E\n";
    REQUIRE_THROWS_AS(GridLoader::loadFromString(text), IoError);
}

TEST_CASE("GridLoader rejects bad characters", "[io][grid]") {
    const auto text =
        "2 2\n"
        "S?\n"
        ".E\n";
    REQUIRE_THROWS_AS(GridLoader::loadFromString(text), IoError);
}

TEST_CASE("GridLoader handles weighted cells", "[io][grid]") {
    const auto text =
        "3 1\n"
        "S5E\n";
    auto g = GridLoader::loadFromString(text);
    REQUIRE(g.weight({0, 1}) == 6);   // '5' → 1+5
}
