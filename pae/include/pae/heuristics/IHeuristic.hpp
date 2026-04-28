#pragma once

#include <string_view>

#include "pae/core/Coord.hpp"

namespace pae::heur {

/// Heuristic interface.
///
/// Contract (all four properties hold for every concrete heuristic;
/// see docs/HEURISTICS.md §1):
///   1. estimate(a, b) >= 0.0
///   2. estimate(a, a) == 0.0
///   3. estimate(a, b) == estimate(b, a)
///   4. estimate(a, b) <= true_cost(a, b) for the documented model
///      (admissibility), and the consistent / monotone property.
///
/// `estimate` is `noexcept` and `const` because it is in A*'s inner loop.
class IHeuristic {
public:
    virtual ~IHeuristic() = default;
    [[nodiscard]] virtual double           estimate(core::Coord from,
                                                    core::Coord to) const noexcept = 0;
    [[nodiscard]] virtual std::string_view name() const noexcept                   = 0;
};

}  // namespace pae::heur
