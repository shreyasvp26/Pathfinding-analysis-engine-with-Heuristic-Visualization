#pragma once

#include "pae/heuristics/IHeuristic.hpp"

namespace pae::heur {

/// Octile distance, a.k.a. "diagonal-step" distance:
///     d_min = min(|dx|, |dy|),  d_max = max(|dx|, |dy|)
///     octile(a, b) = (sqrt(2) - 1) * d_min + d_max
///
/// This is the **tight** admissible heuristic for 8-connectivity with
/// diagonal cost = sqrt(2) and orthogonal cost = 1. It is also
/// admissible (but loose) for 4-connectivity and for 8-connectivity
/// with diagonal cost = 1.
///
/// Compared to the alternatives on 8-conn:
///   - Manhattan  is INADMISSIBLE (overestimates).
///   - Chebyshev  is admissible but assumes diagonal_cost = 1
///                (loose when diagonals cost sqrt(2)).
///   - Euclidean  is admissible but always loose for grid-locked moves.
///   - Octile     is admissible AND tight  ← use this.
///
/// Cost per call: ~5 ALU ops + one `min`/`max`.
class Octile : public IHeuristic {
public:
    [[nodiscard]] double estimate(core::Coord a, core::Coord b) const noexcept override;
    [[nodiscard]] std::string_view name() const noexcept override { return "octile"; }
};

}  // namespace pae::heur
