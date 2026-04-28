#include "pae/algorithms/AStar.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <limits>
#include <queue>
#include <vector>

#include "internal/Reconstruct.hpp"
#include "pae/core/Grid.hpp"
#include "pae/core/Node.hpp"
#include "pae/metrics/Metrics.hpp"
#include "pae/visualization/IVisualizer.hpp"

namespace pae::algo {

namespace {
using clock = std::chrono::steady_clock;

constexpr std::int64_t INF = std::numeric_limits<std::int64_t>::max();

}  // namespace

Result AStar::run(const core::Grid&    grid,
                  const RunConfig&     cfg,
                  viz::IVisualizer*    viz,
                  metrics::Metrics*    metrics) const {
    using core::Cell;
    using core::Coord;
    using core::Node;

    const auto t0  = clock::now();
    const auto V   = grid.width() * grid.height();
    const auto goalIdx  = grid.toIndex(grid.end());

    std::vector<std::int64_t> gScore(static_cast<std::size_t>(V), INF);
    std::vector<std::int32_t> parent(static_cast<std::size_t>(V), -1);

    std::priority_queue<Node, std::vector<Node>, core::NodeFCmp> heap;
    {
        std::vector<Node> backing;
        backing.reserve(static_cast<std::size_t>(V) / 4);
        heap = std::priority_queue<Node, std::vector<Node>, core::NodeFCmp>{
            core::NodeFCmp{}, std::move(backing)};
    }

    const auto startIdx              = grid.toIndex(grid.start());
    gScore[static_cast<std::size_t>(startIdx)] = 0;
    const double h0 = h_.estimate(grid.start(), grid.end());
    heap.push(Node{startIdx, 0, h0, -1});

    if (viz) {
        viz->onSearchStart(grid);
        viz->onEnqueue(grid.start(), h0);
    }

    Result        result;
    std::int64_t  enqCount{1};
    std::int64_t  expCount{0};
    std::size_t   peakOpen{1};

    std::array<Coord, 8> nbrs{};
    int                  nbrCount = 0;

    while (!heap.empty()) {
        peakOpen = std::max(peakOpen, heap.size());
        const Node u = heap.top();
        heap.pop();

        if (u.gCost > gScore[static_cast<std::size_t>(u.cellIndex)]) {
            // Stale entry (lazy decrease-key). Skip without counting.
            continue;
        }
        ++expCount;

        const Coord uc = grid.toCoord(u.cellIndex);
        if (viz) {
            viz->onExpand(uc);
        }

        if (u.cellIndex == goalIdx) {
            result.found     = true;
            result.path      = internal::reconstructPath(grid, parent, goalIdx);
            result.totalCost = gScore[static_cast<std::size_t>(goalIdx)];
            break;
        }

        if (cfg.diagonal) {
            std::array<Coord, 8> n8{};
            grid.neighbors8(uc, n8, nbrCount);
            for (int i = 0; i < nbrCount; ++i) {
                nbrs[static_cast<std::size_t>(i)] = n8[static_cast<std::size_t>(i)];
            }
        } else {
            std::array<Coord, 4> n4{};
            grid.neighbors4(uc, n4, nbrCount);
            for (int i = 0; i < nbrCount; ++i) {
                nbrs[static_cast<std::size_t>(i)] = n4[static_cast<std::size_t>(i)];
            }
        }

        for (int i = 0; i < nbrCount; ++i) {
            const Coord v = nbrs[static_cast<std::size_t>(i)];
            if (grid.unchecked(v) == Cell::Obstacle) {
                continue;
            }
            const auto vIdx       = grid.toIndex(v);
            const auto step       = static_cast<std::int64_t>(grid.weight(v));
            const auto tentativeG = gScore[static_cast<std::size_t>(u.cellIndex)] + step;
            if (tentativeG < gScore[static_cast<std::size_t>(vIdx)]) {
                gScore[static_cast<std::size_t>(vIdx)] = tentativeG;
                parent[static_cast<std::size_t>(vIdx)] = u.cellIndex;
                const double f = static_cast<double>(tentativeG) + h_.estimate(v, grid.end());
                heap.push(Node{vIdx, tentativeG, f, u.cellIndex});
                ++enqCount;
                if (viz) {
                    viz->onEnqueue(v, f);
                }
            }
        }
    }

    const auto t1 = clock::now();

    if (metrics) {
        metrics->nodesExpanded   = expCount;
        metrics->nodesEnqueued   = enqCount;
        metrics->pathLength      = static_cast<std::int64_t>(result.path.size());
        metrics->pathCost        = result.totalCost;
        metrics->wallMicros      =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        metrics->approxPeakBytes =
            static_cast<std::int64_t>(sizeof(Node)) * static_cast<std::int64_t>(peakOpen) +
            static_cast<std::int64_t>(sizeof(std::int64_t)) * V +
            static_cast<std::int64_t>(sizeof(std::int32_t)) * V;
    }
    if (viz) {
        if (result.found) {
            viz->onPathFound(core::Span<const core::Coord>{result.path.data(),
                                                           result.path.size()});
        }
        viz->onSearchComplete(metrics ? *metrics : metrics::Metrics{});
    }

    return result;
}

}  // namespace pae::algo
