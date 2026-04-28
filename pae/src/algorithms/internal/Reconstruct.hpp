#pragma once

#include <cstdint>
#include <vector>

#include "pae/core/Coord.hpp"
#include "pae/core/Grid.hpp"

namespace pae::algo::internal {

/// Walks the parent[] array backward from `goalIdx`, reverses, and
/// returns a path of cell coordinates.
[[nodiscard]] std::vector<core::Coord> reconstructPath(
    const core::Grid&                  grid,
    const std::vector<std::int32_t>&   parent,
    std::int32_t                       goalIdx);

}  // namespace pae::algo::internal
