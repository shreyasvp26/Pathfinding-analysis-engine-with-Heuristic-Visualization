#pragma once

#include "pae/algorithms/IPathfinder.hpp"
#include "pae/heuristics/IHeuristic.hpp"

namespace pae::algo {

/// A* search using `f = g + h`.
///
/// The heuristic is borrowed (non-owning const reference). The caller
/// owns both the heuristic and this AStar object. See LLD §4.2.
class AStar : public IPathfinder {
public:
    explicit AStar(const heur::IHeuristic& h) : h_(h) {}

    [[nodiscard]] Result           run(const core::Grid&    grid,
                                       const RunConfig&     cfg,
                                       viz::IVisualizer*    viz,
                                       metrics::Metrics*    metrics) const override;
    [[nodiscard]] std::string_view name() const noexcept override { return "astar"; }

private:
    const heur::IHeuristic& h_;
};

}  // namespace pae::algo
