#include <catch2/catch_test_macros.hpp>

#include "pae/algorithms/IPathfinder.hpp"
#include "pae/factory/Registry.hpp"
#include "pae/heuristics/IHeuristic.hpp"

using pae::algo::IPathfinder;
using pae::factory::Registry;
using pae::factory::registerAll;
using pae::heur::IHeuristic;

TEST_CASE("Registry knows all built-in algorithms after registerAll", "[registry][factory]") {
    registerAll();
    auto names = Registry<IPathfinder>::instance().names();
    REQUIRE(names.size() >= 3);
    auto contains = [&](std::string_view n) {
        return std::find(names.begin(), names.end(), std::string{n}) != names.end();
    };
    REQUIRE(contains("astar"));
    REQUIRE(contains("dijkstra"));
    REQUIRE(contains("bfs"));
}

TEST_CASE("Registry knows all built-in heuristics after registerAll", "[registry][factory]") {
    registerAll();
    auto names = Registry<IHeuristic>::instance().names();
    auto contains = [&](std::string_view n) {
        return std::find(names.begin(), names.end(), std::string{n}) != names.end();
    };
    REQUIRE(contains("manhattan"));
    REQUIRE(contains("euclidean"));
    REQUIRE(contains("chebyshev"));
}

TEST_CASE("Registry::create throws on unknown name", "[registry][factory]") {
    registerAll();
    REQUIRE_THROWS(Registry<IHeuristic>::instance().create("not-a-heuristic"));
}
