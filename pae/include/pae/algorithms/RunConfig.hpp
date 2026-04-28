#pragma once

#include <cstdint>

namespace pae::algo {

/// Per-call runtime configuration for an `IPathfinder::run`.
struct RunConfig {
    bool          diagonal       = false;  // 8-conn if true, else 4-conn
    int           visualizeEvery = 1;      // emit viz event every N expansions
    std::uint64_t seed           = 0;      // deterministic auxiliary tie-break
};

}  // namespace pae::algo
