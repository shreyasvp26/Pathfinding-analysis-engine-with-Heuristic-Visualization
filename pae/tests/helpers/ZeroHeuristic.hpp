#pragma once

#include "pae/heuristics/IHeuristic.hpp"

namespace pae::tests {

/// Test-only heuristic returning 0 for any pair. Used to verify
/// "A* with h ≡ 0 == Dijkstra".
class ZeroHeuristic : public heur::IHeuristic {
public:
    [[nodiscard]] double estimate(core::Coord, core::Coord) const noexcept override { return 0.0; }
    [[nodiscard]] std::string_view name() const noexcept override { return "zero"; }
};

}  // namespace pae::tests
