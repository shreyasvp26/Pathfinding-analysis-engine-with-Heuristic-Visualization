#pragma once

#include <string_view>

#include "pae/algorithms/Result.hpp"
#include "pae/algorithms/RunConfig.hpp"

namespace pae::core      { class Grid; }
namespace pae::viz       { class IVisualizer; }
namespace pae::metrics   { struct Metrics; }

namespace pae::algo {

/// Pathfinder interface. Implementations: AStar, Dijkstra, BFS.
///
/// `run` is `const` because pathfinders have no mutable state per call;
/// it is therefore trivially re-entrant and safe to call concurrently
/// (post-V1, when benchmarks parallelise).
///
/// `viz` and `metrics` may be nullptr; the algorithm checks once and
/// hoists the branch (or pairs with `NullVisualizer` to skip checks).
class IPathfinder {
public:
    virtual ~IPathfinder() = default;

    [[nodiscard]] virtual Result           run(const core::Grid&    grid,
                                               const RunConfig&     cfg,
                                               viz::IVisualizer*    viz,
                                               metrics::Metrics*    metrics) const = 0;
    [[nodiscard]] virtual std::string_view name() const noexcept                   = 0;
};

}  // namespace pae::algo
