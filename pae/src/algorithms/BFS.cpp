#include "pae/algorithms/BFS.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <queue>
#include <vector>

#include "internal/Reconstruct.hpp"
#include "pae/core/Grid.hpp"
#include "pae/core/Node.hpp"
#include "pae/metrics/Metrics.hpp"
#include "pae/metrics/Rss.hpp"
#include "pae/visualization/IVisualizer.hpp"

namespace pae::algo {

namespace {
using clock = std::chrono::steady_clock;
}  // namespace

Result BFS::run(const core::Grid&    grid,
                const RunConfig&     cfg,
                viz::IVisualizer*    viz,
                metrics::Metrics*    metrics) const {
    using core::Cell;
    using core::Coord;
    using core::Node;

    const auto t0       = clock::now();
    const auto rssStart = metrics::maxResidentBytes();
    const auto V        = grid.width() * grid.height();
    const auto goalIdx  = grid.toIndex(grid.end());

    std::vector<std::uint8_t> visited(static_cast<std::size_t>(V), 0);
    std::vector<std::int32_t> parent(static_cast<std::size_t>(V), -1);

    std::queue<std::int32_t> q;
    const auto startIdx = grid.toIndex(grid.start());
    q.push(startIdx);
    visited[static_cast<std::size_t>(startIdx)] = 1;

    if (viz) {
        viz->onSearchStart(grid);
        viz->onEnqueue(grid.start(), 0.0);
    }

    Result        result;
    std::int64_t  enqCount{1};
    std::int64_t  expCount{0};
    std::size_t   peakOpen{1};

    while (!q.empty()) {
        peakOpen = std::max(peakOpen, q.size());
        const auto uIdx = q.front();
        q.pop();
        ++expCount;

        const Coord uc = grid.toCoord(uIdx);
        if (viz) {
            viz->onExpand(uc);
        }
        if (uIdx == goalIdx) {
            result.found     = true;
            result.path      = internal::reconstructPath(grid, parent, goalIdx);
            result.totalCost = static_cast<std::int64_t>(result.path.size()) - 1;
            break;
        }

        std::array<Coord, 8> nbrs{};
        int                  nbrCount = 0;
        if (cfg.diagonal) {
            grid.neighbors8(uc, nbrs, nbrCount);
        } else {
            std::array<Coord, 4> n4{};
            grid.neighbors4(uc, n4, nbrCount);
            for (int i = 0; i < nbrCount; ++i) {
                nbrs[static_cast<std::size_t>(i)] = n4[static_cast<std::size_t>(i)];
            }
        }

        for (int i = 0; i < nbrCount; ++i) {
            const Coord v    = nbrs[static_cast<std::size_t>(i)];
            const auto  vIdx = grid.toIndex(v);
            if (visited[static_cast<std::size_t>(vIdx)] == 1) {
                continue;
            }
            if (grid.unchecked(v) == Cell::Obstacle) {
                continue;
            }
            visited[static_cast<std::size_t>(vIdx)] = 1;
            parent[static_cast<std::size_t>(vIdx)]  = uIdx;
            q.push(vIdx);
            ++enqCount;
            if (viz) {
                viz->onEnqueue(v, 0.0);
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
            static_cast<std::int64_t>(sizeof(std::int32_t)) * static_cast<std::int64_t>(peakOpen) +
            static_cast<std::int64_t>(sizeof(std::uint8_t)) * V +
            static_cast<std::int64_t>(sizeof(std::int32_t)) * V;

        if constexpr (metrics::trueRssAvailable()) {
            const auto rssEnd = metrics::maxResidentBytes();
            metrics->rssDeltaBytes = rssEnd - rssStart;
        }
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
