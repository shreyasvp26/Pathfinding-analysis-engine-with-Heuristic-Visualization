#include "pae/heuristics/Euclidean.hpp"

#include <cmath>

namespace pae::heur {

double Euclidean::estimate(core::Coord a, core::Coord b) const noexcept {
    const auto dx = static_cast<double>(a.col - b.col);
    const auto dy = static_cast<double>(a.row - b.row);
    return std::sqrt(dx * dx + dy * dy);
}

}  // namespace pae::heur
