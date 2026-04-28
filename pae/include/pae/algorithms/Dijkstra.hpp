#pragma once

#include "pae/algorithms/IPathfinder.hpp"

namespace pae::algo {

/// Dijkstra's algorithm: A* with h ≡ 0, kept as its own class for
/// honest naming, micro-optimisation (no `+ h` arithmetic), and
/// pedagogical clarity. See LLD §4.3.
class Dijkstra : public IPathfinder {
public:
    [[nodiscard]] Result           run(const core::Grid&    grid,
                                       const RunConfig&     cfg,
                                       viz::IVisualizer*    viz,
                                       metrics::Metrics*    metrics) const override;
    [[nodiscard]] std::string_view name() const noexcept override { return "dijkstra"; }
};

}  // namespace pae::algo
