#pragma once

#include "pae/visualization/IVisualizer.hpp"

namespace pae::viz {

/// No-op visualizer. Used by `--visualize none` and benchmark mode
/// (so algorithms can always call through a non-null pointer).
class NullVisualizer final : public IVisualizer {
public:
    void onSearchStart(const core::Grid&)                              override {}
    void onEnqueue(core::Coord, double)                                override {}
    void onExpand(core::Coord)                                         override {}
    void onPathFound(core::Span<const core::Coord>)                    override {}
    void onSearchComplete(const metrics::Metrics&)                     override {}
};

}  // namespace pae::viz
