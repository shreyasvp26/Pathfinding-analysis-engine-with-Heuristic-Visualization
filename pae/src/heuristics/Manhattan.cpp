#include "pae/heuristics/Manhattan.hpp"

#include <cstdlib>

namespace pae::heur {

double Manhattan::estimate(core::Coord a, core::Coord b) const noexcept {
    const auto dx = std::abs(a.col - b.col);
    const auto dy = std::abs(a.row - b.row);
    return static_cast<double>(dx + dy);
}

}  // namespace pae::heur
