#pragma once

#include "pae/algorithms/IPathfinder.hpp"

namespace pae::algo {

/// Breadth-First Search.
///
/// Treats every move as cost 1 (ignores cell weights). Optimal for
/// minimum-step paths on unweighted graphs. See LLD §4.4.
class BFS : public IPathfinder {
public:
    [[nodiscard]] Result           run(const core::Grid&    grid,
                                       const RunConfig&     cfg,
                                       viz::IVisualizer*    viz,
                                       metrics::Metrics*    metrics) const override;
    [[nodiscard]] std::string_view name() const noexcept override { return "bfs"; }
};

}  // namespace pae::algo
