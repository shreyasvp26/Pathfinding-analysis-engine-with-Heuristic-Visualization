#pragma once

#include "pae/heuristics/IHeuristic.hpp"

namespace pae::heur {

/// Manhattan distance: |dx| + |dy|.
///
/// Admissible for: 4-connectivity, unit (or non-negative) per-cell costs.
/// NOT admissible for 8-connectivity (overestimates true cost).
/// Cost per call: ~3 ALU ops.
class Manhattan : public IHeuristic {
public:
    [[nodiscard]] double estimate(core::Coord a, core::Coord b) const noexcept override;
    [[nodiscard]] std::string_view name() const noexcept override { return "manhattan"; }
};

}  // namespace pae::heur
