#pragma once

#include <cstdint>

namespace pae::metrics {

/// Per-run metrics record. POD struct; populated by every algorithm.
///
/// `approxPeakBytes`  is an analytical estimate of the working-set
///                    (open + gScore + parent) — always populated.
/// `rssDeltaBytes`    is `getrusage(RUSAGE_SELF).ru_maxrss` after the run
///                    minus before. Only populated when the build was
///                    compiled with `-DPAE_TRUE_RSS=ON`. Otherwise 0.
/// See docs/PERFORMANCE.md §1.
struct Metrics {
    std::int64_t nodesExpanded   = 0;
    std::int64_t nodesEnqueued   = 0;
    std::int64_t pathLength      = 0;
    std::int64_t pathCost        = 0;
    std::int64_t wallMicros      = 0;
    std::int64_t approxPeakBytes = 0;
    std::int64_t rssDeltaBytes   = 0;

    void reset() noexcept {
        nodesExpanded   = 0;
        nodesEnqueued   = 0;
        pathLength      = 0;
        pathCost        = 0;
        wallMicros      = 0;
        approxPeakBytes = 0;
        rssDeltaBytes   = 0;
    }
};

}  // namespace pae::metrics
