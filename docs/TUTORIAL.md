# Tutorial — adding a new algorithm to PAE

This walks through the **complete** workflow for landing a new
pathfinding algorithm in the Pathfinding Analysis Engine. We'll use a
fictional algorithm called **Greedy Best-First Search (GBFS)** as the
running example. It uses the same heuristic pipeline as A\* but
ignores `g(n)` — perfect for showing the integration points without
distracting algorithmic depth.

> Estimated time: **45 minutes** end-to-end if you've already cloned
> the repo and have a build that passes `ctest`. Read top-to-bottom
> the first time; the same recipe works for Bidirectional A\*, JPS,
> Theta\*, etc.

---

## TL;DR — the 7 touchpoints

To ship `gbfs` you will edit / create these files. Nothing else:

| #  | File                                                    | What you do                                       |
| -- | ------------------------------------------------------- | ------------------------------------------------- |
| 1. | `pae/include/pae/algorithms/Gbfs.hpp`                   | New header. Implements `algo::IPathfinder`.       |
| 2. | `pae/src/algorithms/Gbfs.cpp`                           | Implementation.                                   |
| 3. | `pae/src/algorithms/CMakeLists.txt`                     | Add `Gbfs.cpp` to the source list.                |
| 4. | `pae/src/factory/register_default.cpp`                  | Register the new factory under name `"gbfs"`.     |
| 5. | `pae/tests/test_gbfs.cpp`                               | New test cases (correctness + edge cases).        |
| 6. | `pae/tests/CMakeLists.txt`                              | Add `test_gbfs.cpp` to the test target.           |
| 7. | `pae/src/cli/AppConfig.cpp` + `App.cpp`                 | One-line update: `--algo gbfs` is allowed.        |

Total LOC: ~250 in production, ~100 in tests, plus a brief
[`docs/ALGORITHMS.md`](./ALGORITHMS.md) entry if the algorithm is
non-trivial. **You should not have to touch any other module.** That
is the design property the project is meant to demonstrate.

---

## 1. Create the header — `pae/include/pae/algorithms/Gbfs.hpp`

```cpp
#pragma once

#include "pae/algorithms/IPathfinder.hpp"

namespace pae::heur { class IHeuristic; }

namespace pae::algo {

/// Greedy Best-First Search.
/// Expands the open-set node with the lowest h(n) — i.e. closest to
/// the goal as estimated by the heuristic. NOT optimal, NOT complete
/// on weighted grids, but typically the fastest of the bunch.
class Gbfs : public IPathfinder {
public:
    explicit Gbfs(const heur::IHeuristic& h) noexcept : h_(h) {}

    [[nodiscard]] Result run(const core::Grid&    grid,
                             const RunConfig&     cfg,
                             viz::IVisualizer*    viz,
                             metrics::Metrics*    metrics) const override;
    [[nodiscard]] std::string_view name() const noexcept override { return "gbfs"; }

private:
    const heur::IHeuristic& h_;
};

}  // namespace pae::algo
```

**Why an `IHeuristic&` member?** Same pattern as `AStar`. Constructor
injection so the algorithm doesn't know how its heuristic is stored or
created. Keep `noexcept` on the constructor; the heuristic is borrowed.

**Why `[[nodiscard]]`?** A pathfinder result that's silently dropped
is almost certainly a bug.

---

## 2. Implement it — `pae/src/algorithms/Gbfs.cpp`

The skeleton below is ~80 LOC. The structure is intentionally identical
to `AStar.cpp`; that consistency is part of why new algorithms can be
dropped in.

```cpp
#include "pae/algorithms/Gbfs.hpp"

#include <chrono>
#include <queue>
#include <vector>

#include "internal/Reconstruct.hpp"
#include "pae/core/Grid.hpp"
#include "pae/core/Node.hpp"
#include "pae/heuristics/IHeuristic.hpp"
#include "pae/metrics/Metrics.hpp"
#include "pae/metrics/Rss.hpp"
#include "pae/visualization/IVisualizer.hpp"

namespace pae::algo {

Result Gbfs::run(const core::Grid&    grid,
                 const RunConfig&     cfg,
                 viz::IVisualizer*    viz,
                 metrics::Metrics*    metrics) const {
    using clock = std::chrono::steady_clock;
    const auto t0       = clock::now();
    const auto rssStart = metrics::maxResidentBytes();

    // Open set: priority on h(n) only. Same Node struct as A*; we
    // just leave gCost untouched and let fCost double as h(n).
    auto cmp = [](const core::Node& a, const core::Node& b) {
        return a.fCost > b.fCost;
    };
    std::priority_queue<core::Node, std::vector<core::Node>, decltype(cmp)> open(cmp);

    const auto V = grid.width() * grid.height();
    std::vector<std::int32_t> parent(static_cast<std::size_t>(V), -1);
    std::vector<std::uint8_t> closed(static_cast<std::size_t>(V), 0);

    const auto startIdx = grid.toIndex(grid.start());
    const auto goalIdx  = grid.toIndex(grid.end());
    open.push({startIdx, 0, 0, h_.estimate(grid.start(), grid.end())});

    if (viz) viz->onStart(grid);

    std::int64_t expanded = 0, enqueued = 1;
    while (!open.empty()) {
        const auto u = open.top(); open.pop();
        if (closed[static_cast<std::size_t>(u.cellIndex)]) continue;
        closed[static_cast<std::size_t>(u.cellIndex)] = 1;
        ++expanded;
        if (viz) viz->onExpand(u, grid);

        if (u.cellIndex == goalIdx) break;

        std::array<core::Coord, 8> n8;
        std::array<core::Coord, 4> n4;
        int                        nbrCount = 0;
        if (cfg.diagonal) {
            grid.neighbors8(grid.toCoord(u.cellIndex), n8, nbrCount);
        } else {
            grid.neighbors4(grid.toCoord(u.cellIndex), n4, nbrCount);
        }
        for (int i = 0; i < nbrCount; ++i) {
            const core::Coord nbr = cfg.diagonal ? n8[i] : n4[i];
            if (grid.unchecked(nbr) == core::Cell::Obstacle) continue;
            const auto vIdx = grid.toIndex(nbr);
            if (closed[static_cast<std::size_t>(vIdx)]) continue;
            parent[static_cast<std::size_t>(vIdx)] = u.cellIndex;
            open.push({vIdx, 0, 0, h_.estimate(nbr, grid.end())});
            ++enqueued;
        }
    }

    Result result = internal::reconstruct(grid, parent, goalIdx);
    if (viz) viz->onPath(result.path, grid);

    const auto t1 = clock::now();
    if (metrics) {
        metrics->nodesExpanded = expanded;
        metrics->nodesEnqueued = enqueued;
        metrics->pathLength    = static_cast<std::int64_t>(result.path.size());
        metrics->pathCost      = result.totalCost;
        metrics->wallMicros    =
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        metrics->approxPeakBytes =
            static_cast<std::int64_t>(sizeof(core::Node)) * static_cast<std::int64_t>(open.size()) +
            static_cast<std::int64_t>(sizeof(std::uint8_t)) * V +
            static_cast<std::int64_t>(sizeof(std::int32_t)) * V;
        if constexpr (metrics::trueRssAvailable()) {
            metrics->rssDeltaBytes = metrics::maxResidentBytes() - rssStart;
        }
    }
    if (viz) viz->onFinish(metrics ? *metrics : metrics::Metrics{});
    return result;
}

}  // namespace pae::algo
```

**Five small things to know:**

1. **`internal::reconstruct`** lives in `pae/src/algorithms/internal/Reconstruct.hpp`
   and walks the parent map back from `goalIdx`. Reuse it; do not roll your own.
2. **`grid.neighbors4` / `grid.neighbors8`** write neighbours into a
   stack-allocated `std::array`. Branch on `cfg.diagonal` once per
   pop. Don't iterate `(-1, 0)…(1, 1)` by hand and don't allocate.
3. **`closed[]`** is a flat `vector<uint8_t>`, not `unordered_set`.
   That's a cache + allocator decision documented in
   [`docs/DATA_STRUCTURES.md`](./DATA_STRUCTURES.md). Keep it.
4. **Always update `metrics`.** Any algorithm that returns without
   filling these fields breaks the benchmark sweep.
5. **`viz` may be `nullptr`.** Single null-check per call site.

---

## 3. Wire the build — `pae/src/algorithms/CMakeLists.txt`

```cmake
add_library(pae_algorithms
    AStar.cpp
    Dijkstra.cpp
    BFS.cpp
    Gbfs.cpp                  # ← new
    internal/Reconstruct.cpp
)
target_link_libraries(pae_algorithms
    PUBLIC  pae_headers pae_core pae_heuristics pae_metrics
    PRIVATE pae_warnings
)
```

That's it for build. No new library, no link-time config; it ships
inside `pae_algorithms` because it is one.

---

## 4. Register the factory — `pae/src/factory/register_default.cpp`

```cpp
#include "pae/algorithms/Gbfs.hpp"   // new include
// ...

void registerAll() {
    auto& algoReg = Registry<algo::IPathfinder>::instance();
    algoReg.reg("astar",    /* ... */);
    algoReg.reg("dijkstra", [] { return std::make_unique<algo::Dijkstra>(); });
    algoReg.reg("bfs",      [] { return std::make_unique<algo::BFS>(); });
    // gbfs needs a heuristic, like astar; build it explicitly in App.cpp instead.
    // ...
}
```

> **Stop and think.** A\* is a special case in the registry because it
> needs a heuristic at construction time and the registry erases that.
> If your new algorithm also needs construction-time inputs, follow
> the A\* pattern: leave it out of the algorithm registry and let
> `App::run` handle it. If your algorithm has no constructor inputs
> (like Dijkstra and BFS), register it the lambda way.

---

## 5. Plug it into the CLI — `pae/src/cli/App.cpp`

In the single-run block:

```cpp
if (cfg_.algo == "astar") {
    heuristic = factory::Registry<heur::IHeuristic>::instance().create(cfg_.heuristic);
    algorithm = std::make_unique<algo::AStar>(*heuristic);
} else if (cfg_.algo == "gbfs") {                       // ← new
    heuristic = factory::Registry<heur::IHeuristic>::instance().create(cfg_.heuristic);
    algorithm = std::make_unique<algo::Gbfs>(*heuristic);
} else {
    algorithm = factory::Registry<algo::IPathfinder>::instance().create(cfg_.algo);
}
```

In the benchmark sweep, add `"gbfs"` to `algoNames`. Also update the
help text in `pae/src/cli/AppConfig.cpp`.

---

## 6. Tests — `pae/tests/test_gbfs.cpp`

```cpp
#include <catch2/catch_test_macros.hpp>

#include "pae/algorithms/Gbfs.hpp"
#include "pae/heuristics/Manhattan.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/metrics/Metrics.hpp"

TEST_CASE("Gbfs: finds *a* path on the corridor", "[algo][gbfs]") {
    auto grid = pae::io::GridLoader::loadFromString(
        "5 1\n"
        "S...E\n");
    pae::heur::Manhattan h;
    pae::algo::Gbfs g{h};
    pae::metrics::Metrics m{};
    const auto r = g.run(grid, {}, nullptr, &m);
    REQUIRE(r.found);
    REQUIRE(r.path.size() == 5);
}

TEST_CASE("Gbfs: returns no-path on disconnected map", "[algo][gbfs]") {
    auto grid = pae::io::GridLoader::loadFromString(
        "5 1\n"
        "S##.E\n");
    pae::heur::Manhattan h;
    pae::algo::Gbfs g{h};
    const auto r = g.run(grid, {}, nullptr, nullptr);
    REQUIRE_FALSE(r.found);
}
```

> **Don't put GBFS in `test_cross_algorithm.cpp`.** That file asserts
> _path-cost equivalence_ across algorithms, and GBFS is **not optimal**.
> Adding it there would falsify the suite. This is exactly why we have
> per-algorithm test files.

Add to `pae/tests/CMakeLists.txt`:

```cmake
add_executable(pae_tests
    # ...
    test_gbfs.cpp     # ← new
    # ...
)
```

---

## 7. Verify locally

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug

# Optional: sanity-check on a real map
./pae/build/debug/pae --map pae/maps/maze_20x20.txt --algo gbfs --visualize step
```

If `ctest` reports `100% tests passed`, you've shipped a new algorithm
through the engine without modifying anything outside the seven files
listed at the top.

---

## What to do next

* Add a paragraph to [`docs/ALGORITHMS.md`](./ALGORITHMS.md) describing
  GBFS's complexity and edge cases.
* Update [`docs/FEATURES.md`](./FEATURES.md) with the feature row.
* Run a benchmark and put the headline numbers in your PR description:
  ```bash
  ./pae/build/debug/pae --map pae/maps/maze_50x50.txt --benchmark
  ```
* Open a PR using the `feature_request` template. The `algorithm.agent`
  reviewer auto-claims it via `.github/copilot-instructions.md`.

That's the whole loop.
