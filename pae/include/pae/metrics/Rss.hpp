#pragma once

#include <cstdint>

namespace pae::metrics {

/// Returns the **maximum resident set size** of the current process so
/// far, in bytes. This is an OS-level measurement; it includes the
/// program text, all of `Catch2`, dynamic allocator overhead, and is
/// monotonically non-decreasing across the lifetime of the process.
///
/// On Linux:   `getrusage(RUSAGE_SELF).ru_maxrss` (in KiB) → multiplied by 1024.
/// On macOS:   `getrusage(RUSAGE_SELF).ru_maxrss` (already in bytes).
/// On Windows or when PAE_TRUE_RSS is OFF: returns 0.
///
/// Algorithms call this once at start, once at end, and report the
/// **delta** in `Metrics::approxPeakBytes` IF the build opted in.
/// Otherwise the analytical model in algorithms/AStar.cpp etc. is used.
[[nodiscard]] std::int64_t maxResidentBytes() noexcept;

/// Returns true at compile time iff this build was compiled with
/// `-DPAE_TRUE_RSS=ON` and the platform supports `getrusage`. Algorithms
/// branch on this so the analytical fallback stays fast in default builds.
[[nodiscard]] constexpr bool trueRssAvailable() noexcept {
#if defined(PAE_TRUE_RSS) && (defined(__linux__) || defined(__APPLE__))
    return true;
#else
    return false;
#endif
}

}  // namespace pae::metrics
