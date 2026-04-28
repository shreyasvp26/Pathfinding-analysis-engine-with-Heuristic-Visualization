---
mode: 'agent'
description: 'Add a new IPathfinder end-to-end: header → impl → registry → tests → bench → docs.'
---

Add a new pathfinding algorithm:

${input:algorithmName:Algorithm name in CamelCase — e.g. "BidirectionalAStar"}
${input:lowerName:Lowercase CLI name — e.g. "bdastar"}
${input:rationale:One paragraph: when this algorithm is preferable; references}

This is the canonical extension flow described in
`docs/ARCHITECTURE.md` §9.

## Phase 1 — Plan

1. Confirm the algorithm fits the `IPathfinder` contract
   (`docs/LLD.md` §4.1). If it needs a different return shape, stop and
   open a discussion.
2. Add `F-XXX` row to `docs/FEATURES.md` with status `In Progress`,
   owner `@algorithm`.
3. Branch: `git checkout -b feature/F<id>-${lowerName}`.

## Phase 2 — Implement (`@algorithm`)

1. Header: `pae/include/pae/algorithms/${algorithmName}.hpp`.
   ```cpp
   #pragma once
   #include "pae/algorithms/IPathfinder.hpp"

   namespace pae::algo {
   class ${algorithmName} : public IPathfinder {
   public:
       Result run(const core::Grid&, const RunConfig&,
                  viz::IVisualizer*, metrics::Metrics*) const override;
       std::string_view name() const noexcept override { return "${lowerName}"; }
   };
   }
   ```
2. Impl: `pae/src/algorithms/${algorithmName}.cpp`.
   - Follow the skeleton in `docs/ALGORITHMS.md` §1.
   - Reuse `pae/src/algorithms/internal/Reconstruct.hpp`.
   - No allocations in the inner loop.
3. Update `pae/CMakeLists.txt` (or the `pae_algorithms` library target)
   to include the new source file.

## Phase 3 — Register (`@core`)

In `pae/src/factory/register_default.cpp`:

```cpp
Registry<IPathfinder>::instance().reg(
    "${lowerName}",
    [] { return std::make_unique<algo::${algorithmName}>(); });
```

## Phase 4 — Test (`@qa`)

1. New file: `pae/tests/test_${lowerName}.cpp`. Cover EC-01 → EC-08
   from `docs/TESTING.md` §3.
2. Equivalence test: on uniform-cost maps, the new algorithm's
   `totalCost` matches Dijkstra's.
3. Optimality on weighted maps (if the algorithm claims optimality).
4. Add the new algorithm to the parametrised cross-algorithm test in
   `pae/tests/test_cross_algorithm.cpp`.

## Phase 5 — Benchmark (`@perf`)

1. Add `${lowerName}` to the benchmark sweep set in
   `pae/benchmarks/bench_pathfinders.cpp`.
2. Run the benchmark on the canonical map suite and commit a baseline
   row to `pae/benchmarks/baselines/<runner>/` if applicable.
3. Paste the comparison table into the PR description.

## Phase 6 — Document

1. Append a new section to `docs/ALGORITHMS.md` (Pseudo-code,
   properties, edge cases — same template as §2/§3/§4).
2. Update the comparison matrix in `docs/ALGORITHMS.md` §5.
3. Update `docs/FEATURES.md` row → `Completed`.
4. `docs/CHANGELOG.md`: `feat(algo): add ${algorithmName} ([#PR])`.

## Phase 7 — Ship

1. Commit message: `feat(algo): add ${algorithmName}`.
2. PR via template; reviewers `@algorithm` + `@qa` + `@perf`.
3. CI green; squash merge.

End-to-end the change should touch:

- 1 header, 1 impl in `pae/include/pae/algorithms/` and `pae/src/algorithms/`,
- 1 line in `pae/src/factory/register_default.cpp`,
- 1 test file in `pae/tests/`,
- 1 entry in `pae/benchmarks/bench_pathfinders.cpp`,
- 4 docs files (`ALGORITHMS.md`, `FEATURES.md`, `CHANGELOG.md`, optionally `ROADMAP.md`).

**Nothing in `Grid`, `IHeuristic`, `IVisualizer`, or the CLI is touched.**
That's the contract.
