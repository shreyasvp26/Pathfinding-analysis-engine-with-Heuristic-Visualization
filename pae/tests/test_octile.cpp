#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <cmath>

#include "pae/heuristics/Chebyshev.hpp"
#include "pae/heuristics/Manhattan.hpp"
#include "pae/heuristics/Octile.hpp"

using Catch::Approx;
using pae::core::Coord;
using pae::heur::Chebyshev;
using pae::heur::Manhattan;
using pae::heur::Octile;

TEST_CASE("Octile: zero on diagonal, symmetric, non-negative",
          "[heuristic][octile][property]") {
    Octile h;
    auto r1 = GENERATE(range(0, 12));
    auto c1 = GENERATE(range(0, 12));
    auto r2 = GENERATE(range(0, 12));
    auto c2 = GENERATE(range(0, 12));
    Coord a{r1, c1}, b{r2, c2};
    REQUIRE(h.estimate(a, b) >= 0.0);
    REQUIRE(h.estimate(a, b) == Approx(h.estimate(b, a)).margin(1e-12));
    REQUIRE(h.estimate(a, a) == 0.0);
}

TEST_CASE("Octile: closed-form values are correct",
          "[heuristic][octile]") {
    Octile h;
    // Pure orthogonal: octile == max(dx, dy).
    REQUIRE(h.estimate({0, 0}, {0, 5}) == Approx(5.0));
    REQUIRE(h.estimate({0, 0}, {5, 0}) == Approx(5.0));
    // Pure diagonal: octile == sqrt(2) * d.
    REQUIRE(h.estimate({0, 0}, {3, 3}) == Approx(3.0 * std::sqrt(2.0)));
    // Mixed: dx=4, dy=3 → 3 diagonals + 1 orthogonal = 3*sqrt(2) + 1.
    REQUIRE(h.estimate({0, 0}, {3, 4}) == Approx(3.0 * std::sqrt(2.0) + 1.0));
}

TEST_CASE("Octile is tighter than Chebyshev on diagonals (both admissible 8-conn)",
          "[heuristic][octile][admissibility]") {
    // For 8-connectivity with diag cost = sqrt(2):
    //   chebyshev(a, b) = max(dx, dy)             ← admissible if diag=1, LOOSE if diag=sqrt(2)
    //   octile(a, b)    = (sqrt(2)-1)*min + max   ← TIGHT for diag=sqrt(2)
    // So octile(a,b) >= chebyshev(a,b) on every off-axis pair.
    Chebyshev cheb;
    Octile    oct;
    REQUIRE(oct.estimate({0, 0}, {3, 3}) > cheb.estimate({0, 0}, {3, 3}));
    REQUIRE(oct.estimate({0, 0}, {3, 4}) > cheb.estimate({0, 0}, {3, 4}));
    // On pure orthogonal, the two agree.
    REQUIRE(oct.estimate({0, 0}, {0, 7}) == Approx(cheb.estimate({0, 0}, {0, 7})));
}

TEST_CASE("Octile is admissible vs Manhattan for 4-conn unit cost",
          "[heuristic][octile][admissibility]") {
    // For 4-conn unit cost the true cost is dx + dy (= manhattan).
    // Octile must NOT exceed manhattan for any pair, otherwise A*+octile+4-conn
    // would lose its optimality guarantee.
    Manhattan man;
    Octile    oct;
    auto r1 = GENERATE(range(0, 6));
    auto c1 = GENERATE(range(0, 6));
    auto r2 = GENERATE(range(0, 6));
    auto c2 = GENERATE(range(0, 6));
    REQUIRE(oct.estimate({r1, c1}, {r2, c2})
            <= Approx(man.estimate({r1, c1}, {r2, c2})).margin(1e-9));
}
