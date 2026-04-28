#include <catch2/catch_test_macros.hpp>

#include "pae/algorithms/AStar.hpp"
#include "pae/heuristics/Manhattan.hpp"
#include "pae/io/GridLoader.hpp"

using pae::algo::AStar;
using pae::algo::RunConfig;
using pae::heur::Manhattan;
using pae::io::GridLoader;
using pae::metrics::Metrics;

TEST_CASE("A* + Manhattan: corridor optimal", "[astar][algo]") {
    auto g = GridLoader::loadFromString(
        "5 1\n"
        "S...E\n");
    Manhattan h;
    AStar     a{h};
    Metrics   m{};
    const auto r = a.run(g, RunConfig{}, nullptr, &m);
    REQUIRE(r.found);
    REQUIRE(r.totalCost == 4);
}

TEST_CASE("A*: no-path map returns not found", "[astar][algo]") {
    auto g = GridLoader::loadFromString(
        "3 1\n"
        "S#E\n");
    Manhattan h;
    AStar     a{h};
    Metrics   m{};
    const auto r = a.run(g, RunConfig{}, nullptr, &m);
    REQUIRE_FALSE(r.found);
}
