#include <catch2/catch_test_macros.hpp>

#include "pae/algorithms/BFS.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/metrics/Metrics.hpp"

using pae::algo::BFS;
using pae::algo::RunConfig;
using pae::io::GridLoader;
using pae::metrics::Metrics;

TEST_CASE("BFS: corridor finds linear path", "[bfs][algo]") {
    auto g = GridLoader::loadFromString(
        "5 1\n"
        "S...E\n");
    BFS bfs;
    Metrics m{};
    const auto r = bfs.run(g, RunConfig{}, nullptr, &m);
    REQUIRE(r.found);
    REQUIRE(r.path.size() == 5);
    REQUIRE(r.totalCost   == 4);
    REQUIRE(m.pathLength  == 5);
}

TEST_CASE("BFS: no path returns Result{found=false}", "[bfs][algo]") {
    auto g = GridLoader::loadFromString(
        "3 1\n"
        "S#E\n");
    BFS bfs;
    Metrics m{};
    const auto r = bfs.run(g, RunConfig{}, nullptr, &m);
    REQUIRE_FALSE(r.found);
    REQUIRE(r.path.empty());
    REQUIRE(r.totalCost == 0);
}
