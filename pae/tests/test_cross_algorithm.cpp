#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>

#include "helpers/ZeroHeuristic.hpp"
#include "pae/algorithms/AStar.hpp"
#include "pae/algorithms/Dijkstra.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/metrics/Metrics.hpp"

using pae::algo::AStar;
using pae::algo::Dijkstra;
using pae::algo::RunConfig;
using pae::io::GridLoader;
using pae::metrics::Metrics;
using pae::tests::ZeroHeuristic;

// NOTE: ASCII-only TEST_CASE name on purpose. ctest passes the case name
// as an argv string to the test binary, and Windows argv is decoded with
// the system code page (CP-1252 / etc.), which mangles non-ASCII bytes
// before Catch2 sees them. We hit this with the unicode equivalence sign
// `≡` (U+2261); rename → ASCII fixed it.
TEST_CASE("A* with h equal 0 matches Dijkstra path cost", "[cross][algo]") {
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
