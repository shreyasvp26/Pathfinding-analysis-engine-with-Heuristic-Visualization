#include "pae/heuristics/Octile.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace pae::heur {

namespace {
// sqrt(2) - 1, computed at compile time.
constexpr double kDiagBonus = 0.41421356237309504880;
}  // namespace

double Octile::estimate(core::Coord a, core::Coord b) const noexcept {
    const auto dx = std::abs(a.col - b.col);
    const auto dy = std::abs(a.row - b.row);
    const auto dmin = std::min(dx, dy);
    const auto dmax = std::max(dx, dy);
    return kDiagBonus * static_cast<double>(dmin) + static_cast<double>(dmax);
}

}  // namespace pae::heur
