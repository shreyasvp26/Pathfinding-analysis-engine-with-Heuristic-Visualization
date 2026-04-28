#pragma once

#include <cstdint>

namespace pae::core {

/// Algorithm-side search-frontier record.
///
/// 24 bytes on most ABIs (no padding). Used as the value type inside
/// `std::priority_queue<Node, std::vector<Node>, NodeFCmp>`.
struct Node {
    std::int32_t cellIndex{-1};   // row-major grid index
    std::int64_t gCost{0};        // exact cost from start
    double       fCost{0.0};      // gCost + heuristic (or just gCost for Dijkstra)
    std::int32_t parentCell{-1};
};

/// Min-heap comparator on `fCost`, secondarily max-heap on `gCost`.
///
/// `priority_queue` is a max-heap on the comparator's `<`. We invert
/// so the heap becomes a min-heap on `fCost`. Tie-breaking prefers
/// larger `gCost` (deeper toward goal under admissible h), then
/// smaller cell index for full determinism.
struct NodeFCmp {
    bool operator()(const Node& a, const Node& b) const noexcept {
        if (a.fCost != b.fCost) {
            return a.fCost > b.fCost;
        }
        if (a.gCost != b.gCost) {
            return a.gCost < b.gCost;
        }
        return a.cellIndex > b.cellIndex;
    }
};

}  // namespace pae::core
