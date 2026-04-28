#pragma once

#include "pae/core/Coord.hpp"
#include "pae/core/Span.hpp"
#include "pae/metrics/Metrics.hpp"

namespace pae::core { class Grid; }

namespace pae::viz {

/// Visualization interface. Algorithms emit events; the implementation
/// decides whether (and how) to render. See docs/LLD.md §5.
class IVisualizer {
public:
    virtual ~IVisualizer() = default;

    virtual void onSearchStart(const core::Grid& grid)                       = 0;
    virtual void onEnqueue(core::Coord c, double f)                          = 0;
    virtual void onExpand(core::Coord c)                                     = 0;
    virtual void onPathFound(core::Span<const core::Coord> path)             = 0;
    virtual void onSearchComplete(const metrics::Metrics& m)                 = 0;
};

}  // namespace pae::viz
