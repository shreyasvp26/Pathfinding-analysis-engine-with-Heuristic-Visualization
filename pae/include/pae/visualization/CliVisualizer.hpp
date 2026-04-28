#pragma once

#include <chrono>
#include <cstdint>
#include <ostream>
#include <vector>

#include "pae/visualization/IVisualizer.hpp"
#include "pae/visualization/VizMode.hpp"

namespace pae::viz {

struct CliConfig {
    VizMode mode      = VizMode::Step;
    bool    color     = true;
    int     fpsCap    = 30;
    bool    clearTerm = true;
};

/// ANSI / ASCII terminal visualizer.
///
/// `Step` mode redraws after each `onExpand`, throttled to `fpsCap`
/// frames per second. `Final` redraws once on `onPathFound`.
/// `None` is a no-op (use NullVisualizer instead in hot paths).
///
/// Output stream is injectable so tests can capture into a stringstream.
class CliVisualizer : public IVisualizer {
public:
    CliVisualizer(CliConfig cfg, std::ostream& out);

    void onSearchStart(const core::Grid& grid)                              override;
    void onEnqueue(core::Coord c, double f)                                 override;
    void onExpand(core::Coord c)                                            override;
    void onPathFound(core::Span<const core::Coord> path)                    override;
    void onSearchComplete(const metrics::Metrics& m)                        override;

private:
    void render() const;

    CliConfig                                cfg_;
    std::ostream&                            out_;
    const core::Grid*                        grid_{nullptr};
    std::vector<std::uint8_t>                openSet_;
    std::vector<std::uint8_t>                closedSet_;
    std::vector<core::Coord>                 finalPath_;
    std::chrono::steady_clock::time_point    lastFrame_{};
};

}  // namespace pae::viz
