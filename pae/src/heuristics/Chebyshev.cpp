#include "pae/heuristics/Chebyshev.hpp"

#include <algorithm>
#include <cstdlib>

namespace pae::heur {

double Chebyshev::estimate(core::Coord a, core::Coord b) const noexcept {
    const auto dx = std::abs(a.col - b.col);
    const auto dy = std::abs(a.row - b.row);
    return static_cast<double>(std::max(dx, dy));
}

}  // namespace pae::heur
