#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "helpers/ZeroHeuristic.hpp"
#include "pae/algorithms/AStar.hpp"
#include "pae/algorithms/Dijkstra.hpp"
#include "pae/io/GridLoader.hpp"

using pae::algo::AStar;
using pae::algo::Dijkstra;
using pae::algo::RunConfig;
using pae::io::GridLoader;
using pae::metrics::Metrics;
using pae::tests::ZeroHeuristic;

TEST_CASE("A* with h≡0 matches Dijkstra path cost", "[cross][algo]") {
    const auto map = GENERATE(
        std::string{"5 1\nS...E\n"},
        std::string{"3 3\nS..\n.#.\n..E\n"},
        std::string{"4 2\nS..E\n....\n"}
    );
    auto g = GridLoader::loadFromString(map);

    ZeroHeuristic h0;
    AStar         a{h0};
    Dijkstra      d;

    Metrics ma{}, md{};
    const auto ra = a.run(g, RunConfig{}, nullptr, &ma);
    const auto rd = d.run(g, RunConfig{}, nullptr, &md);

    REQUIRE(ra.found     == rd.found);
    REQUIRE(ra.totalCost == rd.totalCost);
}
