#pragma once

#include "pae/heuristics/IHeuristic.hpp"

namespace pae::heur {

/// Euclidean distance: sqrt(dx^2 + dy^2).
///
/// Admissible for: any movement model (it never overestimates the
/// straight-line distance). For grid-locked 4- or 8-conn, it is
/// admissible but loose, so A* expands more nodes than tighter
/// heuristics.
/// Cost per call: ~10–14 ns (one sqrt).
class Euclidean : public IHeuristic {
public:
    [[nodiscard]] double estimate(core::Coord a, core::Coord b) const noexcept override;
    [[nodiscard]] std::string_view name() const noexcept override { return "euclidean"; }
};

}  // namespace pae::heur
