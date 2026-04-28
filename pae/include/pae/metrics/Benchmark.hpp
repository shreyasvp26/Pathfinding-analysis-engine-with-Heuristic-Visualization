#pragma once

#include <string>
#include <vector>

#include "pae/core/Span.hpp"
#include "pae/metrics/Metrics.hpp"

namespace pae::core { class Grid; }

namespace pae::metrics {

struct BenchmarkRun {
    std::string algoName;
    std::string heuristicName;   // empty for non-A* algorithms
    Metrics     metrics;
    int         repIndex{0};
};

class Benchmark {
public:
    struct Config {
        int  repetitions = 30;
        int  warmup      = 3;
        bool useNullViz  = true;
    };

    /// Sweeps the cross product (algos × heuristics) over a grid.
    /// Skips combos that are not meaningful (e.g. dijkstra+manhattan).
    static std::vector<BenchmarkRun> sweep(
        const core::Grid&                  grid,
        core::Span<const std::string>      algoNames,
        core::Span<const std::string>      heuristicNames,
        Config                             cfg);
};

}  // namespace pae::metrics
