#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include "pae/heuristics/Chebyshev.hpp"

using pae::core::Coord;
using pae::heur::Chebyshev;

TEST_CASE("Chebyshev: zero on diagonal, symmetric, non-negative",
          "[heuristic][chebyshev][property]") {
    Chebyshev h;
    auto r1 = GENERATE(range(0, 12));
    auto c1 = GENERATE(range(0, 12));
    auto r2 = GENERATE(range(0, 12));
    auto c2 = GENERATE(range(0, 12));
    Coord a{r1, c1}, b{r2, c2};
    REQUIRE(h.estimate(a, b) >= 0.0);
    REQUIRE(h.estimate(a, b) == h.estimate(b, a));
    REQUIRE(h.estimate(a, a) == 0.0);
}

TEST_CASE("Chebyshev equals max(|dx|, |dy|)", "[heuristic][chebyshev]") {
    Chebyshev h;
    REQUIRE(h.estimate({0, 0}, {3, 4}) == 4.0);
    REQUIRE(h.estimate({2, 5}, {6, 7}) == 4.0);
    REQUIRE(h.estimate({1, 1}, {1, 1}) == 0.0);
}
