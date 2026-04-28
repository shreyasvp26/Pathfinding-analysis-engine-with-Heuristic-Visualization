#include "Reconstruct.hpp"

#include <algorithm>

namespace pae::algo::internal {

std::vector<core::Coord> reconstructPath(
    const core::Grid&              grid,
    const std::vector<std::int32_t>& parent,
    std::int32_t                   goalIdx) {
    std::vector<core::Coord> path;
    path.reserve(64);
    std::int32_t cur = goalIdx;
    while (cur != -1) {
        path.push_back(grid.toCoord(cur));
        cur = parent[static_cast<std::size_t>(cur)];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

}  // namespace pae::algo::internal
