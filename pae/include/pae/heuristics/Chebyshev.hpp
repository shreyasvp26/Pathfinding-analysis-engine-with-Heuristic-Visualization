#pragma once

#include "pae/heuristics/IHeuristic.hpp"

namespace pae::heur {

/// Chebyshev distance: max(|dx|, |dy|).
///
/// Admissible AND tight for 8-connectivity with diagonal cost == 1
/// (chess-king movement). Admissible (but loose) for 4-conn.
/// Cost per call: ~3 ALU ops.
class Chebyshev : public IHeuristic {
public:
    [[nodiscard]] double estimate(core::Coord a, core::Coord b) const noexcept override;
    [[nodiscard]] std::string_view name() const noexcept override { return "chebyshev"; }
};

}  // namespace pae::heur
