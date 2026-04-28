#pragma once

#include <cstdint>
#include <vector>

#include "pae/core/Coord.hpp"

namespace pae::algo {

/// Outcome of a single `IPathfinder::run`.
struct Result {
    bool                          found{false};
    std::vector<core::Coord>      path;
    std::int64_t                  totalCost{0};
};

}  // namespace pae::algo
