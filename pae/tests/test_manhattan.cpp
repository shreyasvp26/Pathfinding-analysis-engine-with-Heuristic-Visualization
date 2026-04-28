#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include "pae/heuristics/Manhattan.hpp"

using pae::core::Coord;
using pae::heur::Manhattan;

TEST_CASE("Manhattan: zero on diagonal, symmetric, non-negative",
          "[heuristic][manhattan][property]") {
    Manhattan h;
    auto r1 = GENERATE(range(0, 16));
    auto c1 = GENERATE(range(0, 16));
    auto r2 = GENERATE(range(0, 16));
    auto c2 = GENERATE(range(0, 16));
    Coord a{r1, c1}, b{r2, c2};
    REQUIRE(h.estimate(a, b) >= 0.0);
    REQUIRE(h.estimate(a, b) == h.estimate(b, a));
    REQUIRE(h.estimate(a, a) == 0.0);
}

TEST_CASE("Manhattan equals dx + dy", "[heuristic][manhattan]") {
    Manhattan h;
    REQUIRE(h.estimate({0, 0}, {3, 4}) == 7.0);
    REQUIRE(h.estimate({2, 5}, {2, 5}) == 0.0);
    REQUIRE(h.estimate({1, 1}, {0, 0}) == 2.0);
}
