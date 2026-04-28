#include <catch2/catch_test_macros.hpp>

#include "pae/algorithms/Dijkstra.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/metrics/Metrics.hpp"

using pae::algo::Dijkstra;
using pae::algo::RunConfig;
using pae::io::GridLoader;
using pae::metrics::Metrics;

TEST_CASE("Dijkstra: prefers lighter path on weighted grid", "[dijkstra][algo]") {
    // Two routes from S to E: top row has heavy weight-10 cells ('9'),
    // lower rows are normal weight-1. Dijkstra must pick the detour.
    auto g = GridLoader::loadFromString(
        "5 3\n"
        "S99.E\n"
        ".....\n"
        ".....\n");
    Dijkstra d;
    Metrics  m{};
    const auto r = d.run(g, RunConfig{}, nullptr, &m);
    REQUIRE(r.found);
    // Heavy route cost = 1+10+10+1+1 = 23; detour route stays ≤ 8.
    REQUIRE(r.totalCost <= 8);
}

TEST_CASE("Dijkstra: corridor optimal (uniform weights)", "[dijkstra][algo]") {
    auto g = GridLoader::loadFromString(
        "5 1\n"
        "S...E\n");
    Dijkstra d;
    Metrics  m{};
    const auto r = d.run(g, RunConfig{}, nullptr, &m);
    REQUIRE(r.found);
    REQUIRE(r.totalCost == 4);
}
