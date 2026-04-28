#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include "pae/heuristics/Euclidean.hpp"

using Catch::Approx;
using pae::core::Coord;
using pae::heur::Euclidean;

TEST_CASE("Euclidean: zero on diagonal, symmetric, non-negative",
          "[heuristic][euclidean][property]") {
    Euclidean h;
    auto r1 = GENERATE(range(0, 8));
    auto c1 = GENERATE(range(0, 8));
    auto r2 = GENERATE(range(0, 8));
    auto c2 = GENERATE(range(0, 8));
    Coord a{r1, c1}, b{r2, c2};
    REQUIRE(h.estimate(a, b) >= 0.0);
    REQUIRE(h.estimate(a, b) == Approx(h.estimate(b, a)).margin(1e-12));
    REQUIRE(h.estimate(a, a) == 0.0);
}

TEST_CASE("Euclidean equals sqrt(dx^2 + dy^2)", "[heuristic][euclidean]") {
    Euclidean h;
    REQUIRE(h.estimate({0, 0}, {3, 4}) == Approx(5.0));
    REQUIRE(h.estimate({0, 0}, {0, 7}) == Approx(7.0));
    REQUIRE(h.estimate({1, 1}, {1, 1}) == 0.0);
}
