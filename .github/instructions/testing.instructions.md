---
description: 'Auto-applies when editing the test suite.'
applyTo: 'pae/tests/**'
---

# Testing — coding rules

## Source of truth

- `docs/TESTING.md` (master).
- `docs/REQUIREMENTS.md` §NFR-3.

## Framework

- **Catch2 v3** (header v3 single-include). `#include <catch2/catch_test_macros.hpp>`.
- For property generators: `#include <catch2/generators/catch_generators.hpp>`.

## File / case naming

- File: `test_<area>.cpp` (e.g., `test_astar.cpp`, `test_manhattan.cpp`).
- Case: `TEST_CASE("<sentence>", "[<labels>]")`.
- Labels: `[grid]`, `[algo]`, `[heuristic]`, `[viz]`, `[cross]`,
  `[property]`, `[perf]`. CI uses these to filter (`ctest -L heuristic`).

## Determinism

- No `std::time(nullptr)`, no `std::random_device`. Use the `seed`
  argument of `RunConfig` if randomness is needed.
- No `std::chrono::system_clock`.
- `std::mt19937{seed}` with a hard-coded seed.
- Snapshot files use LF line endings; ensured by `.gitattributes`.

## Helpers

`pae/tests/helpers/`:

- `TestGrid.hpp` — small builders for fixture grids.
  ```cpp
  Grid makeOpenGrid(int w, int h);
  Grid makeWallBetween(Coord s, Coord e);
  ```
- `ZeroHeuristic.hpp` — `IHeuristic` returning 0; for cross-algo
  tests.

## Required test patterns per module

### `Grid` / `GridLoader`

- Construction with bad inputs throws (`IoError`).
- `inBounds` exhaustive on a small grid.
- `neighbors4` / `neighbors8` correct count (4 or 8 in interior;
  fewer on edges/corners).

### Heuristics

- Property battery: non-negative, symmetric, goal-zero.
- Sampled admissibility against true cost on a uniform grid.

### Algorithms

- EC-01 → EC-08 from `docs/TESTING.md` §3.
- Cross-algorithm: A\* with `ZeroHeuristic` matches Dijkstra
  byte-for-byte on every fixture.
- Path validity: `path.front() == grid.start()`, `path.back() == grid.end()`,
  every adjacent pair is a 4- (or 8-) neighbour, every cell on the
  path is non-obstacle.
- Metrics consistency: `pathLength == path.size()`,
  `pathCost == sum of weights`, `nodesEnqueued >= nodesExpanded`.

### Visualizer

- Snapshot match against `fixtures/snapshots/<name>.txt` (`--no-color`).
- Two consecutive runs produce byte-equal output.
- `NullVisualizer` produces no output (assert empty stream).

### Registry / factory

- Unknown name throws `pae::UnknownNameError`.
- Registered name produces an instance of the expected concrete type
  (test via `dynamic_cast`).
- `names()` returns the registered names, sorted.

## Hard rules

1. **No `std::cout`** in tests. Use Catch2 `INFO` / `WARN` /
   `CAPTURE`.
2. **No globals.** Each `TEST_CASE` constructs its own state.
3. **No `sleep_for`.** Anywhere. Including the visualizer tests
   (use the `fpsCap = 0` test mode that disables throttling).
4. **No retries.** Flaky tests are bugs.
5. **No skipped tests** without a `B###` link in a comment above the
   `TEST_CASE`.

## CMake registration

In `pae/tests/CMakeLists.txt`:

```cmake
add_executable(pae_tests
    test_grid.cpp
    test_grid_loader.cpp
    # …
)
target_link_libraries(pae_tests
    PRIVATE
        Catch2::Catch2WithMain
        pae_core
        pae_io
        pae_heuristics
        pae_algorithms
        pae_visualization
        pae_metrics
        pae_factory
)
target_include_directories(pae_tests PRIVATE helpers)

include(Catch)
catch_discover_tests(pae_tests
    PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        LABELS            "<labels>"
)
```
