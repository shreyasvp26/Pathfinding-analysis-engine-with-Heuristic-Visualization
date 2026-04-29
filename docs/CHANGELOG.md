# Changelog — Pathfinding Analysis Engine

All notable changes to this project are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Conventions (parity with `ssm_calender` and `Jyotish AI`):
- `[Unreleased]` is the holding pen; entries collect there during the
  cycle, then the heading is renamed to the new tag at release time.
- Entry shape: `- type(scope): one-sentence change ([#PR](url))`.
- Types: `feat`, `fix`, `docs`, `refactor`, `perf`, `test`, `build`,
  `ci`, `chore`.

---

## [Unreleased]

### Added
- ci+test: real perf-budget ctest cases — `test_perf_budget.cpp` (F-404) with three `[perf]`-tagged cases that run `Benchmark::sweep` on `maze_20x20`, `open_arena_50x50`, and `maze_50x50` and assert order-of-magnitude wall-time + path-cost budgets. Compiled only with `PAE_PERF_BUDGET=ON` (e.g. `cmake --preset perf`); the `perf-budget` job in `ci.yml` now actually has tests to run via `ctest -L perf`.
- ci: new `readme-smoke` job (F-504) that runs the README quick-start commands verbatim — `cmake --preset debug`, `cmake --build --preset debug`, `ctest --preset debug`, plus a real CLI run and a real `--benchmark` invocation. Catches drift between docs and reality on every `main` push.
- feat(heuristics): `Octile` distance heuristic — the **tight** admissible heuristic for 8-connectivity with diagonal cost √2. Registered under name `"octile"` and wired into the benchmark sweep.
- feat(metrics): true OS-level RSS probe (`pae::metrics::maxResidentBytes`, `getrusage(RUSAGE_SELF)`). Gated behind the new `PAE_TRUE_RSS` CMake option (default OFF, no-op on Windows). When ON, every algorithm fills `Metrics::rssDeltaBytes` and the CLI summary + JSON / CSV reports include the RSS delta.
- feat(maps): `pae/maps/maze_50x50.txt` (recursive-backtracker maze, seed 42) and `pae/maps/open_arena_50x50.txt` (random-arena, density 0.30, seed 1) so benchmark numbers exercise enough state to actually matter.
- chore(scripts): `pae/scripts/gen_maze.py` — deterministic, parameterised map generator with `maze` and `arena` modes.
- build: `pae/CMakePresets.json` with `debug`, `release`, `san`, `perf`, `rss` configure presets and matching build / test / workflow presets, so contributors don't memorise flag combos.
- design: `design/diagrams.md` plus four `.mmd` source files (`architecture`, `class_diagram`, `sequence_run`, `sequence_benchmark`). GitHub renders them inline; no Mermaid CLI required to read them. `design/README.md` updated to point at the new artifacts.
- docs: `docs/TUTORIAL.md` — end-to-end walkthrough for adding a new algorithm to the engine, with the running GBFS example and the seven-touchpoint integration recipe.
- test: snapshot-test cases for `CliVisualizer` that lock the rendered grid format on a corridor map and an obstacle map (3 new test cases total, paired with comments explaining what to update if a glyph or row separator changes).
- test: 4 new property + closed-form tests for the `Octile` heuristic (admissibility vs Manhattan on 4-conn, tightness vs Chebyshev on 8-conn, symmetry, zero-on-self).
- docs: comprehensive engineering blueprint — `ARCHITECTURE.md`, `LLD.md`, `ALGORITHMS.md`, `HEURISTICS.md`, `DATA_STRUCTURES.md`, `TESTING.md`, `PERFORMANCE.md`, `EXTENSIONS.md`, `ROADMAP.md`, `IMPLEMENTATION_PLAN.md`, `FOLDER_STRUCTURE.md`, `REQUIREMENTS.md`, `FEATURES.md`, `BUGS.md`.
- docs: `AGENTS.md` orchestration map (Core, Algorithm, Heuristic, Visualization, Performance, QA, Build).
- chore: repo skeleton — `README.md`, `SECURITY.md`, `LICENSE` (MIT), `.gitignore`, `.gitattributes`, `.editorconfig`, `.clang-format`, `.clang-tidy`, `.pre-commit-config.yaml`.
- build: top-level `CMakeLists.txt` delegating to `pae/CMakeLists.txt`.
- build: `.github/` scaffolding — `copilot-instructions.md`, `SETUP_GITHUB.md`, agent personas, instruction files, prompt library, CI workflows.
- feat(core): `Coord`, `Cell`, `Node` value types; `Grid` (immutable, row-major, with bounds checks and 4-/8-conn neighbour iteration); C++17 `Span<T>` polyfill.
- feat(io): `GridLoader` (ASCII grid parser with comment header, weighted cells, and `IoError` reporting).
- feat(heuristics): `IHeuristic` interface plus `Manhattan`, `Euclidean`, `Chebyshev` implementations.
- feat(algorithms): `IPathfinder` interface plus `AStar`, `Dijkstra`, `BFS` implementations (4- and 8-conn, lazy decrease-key, exposes `Result` + `Metrics`).
- feat(visualization): `IVisualizer` interface plus `CliVisualizer` (ANSI colour, fps-capped step mode, final-frame mode) and `NullVisualizer`.
- feat(metrics): `Metrics` POD; `Benchmark::sweep` producing `BenchmarkRun[]`; `Report` writer (table / CSV / JSON).
- feat(factory): templated `Registry<T>` and `registerAll()` composition root for both algorithms and heuristics.
- feat(cli): `pae` binary with `--map / --algo / --heuristic / --visualize / --diagonal / --no-color / --benchmark / --seed / --help / --version` and a separate `pae_bench` harness.
- feat(maps): six sample grids in `pae/maps/` (`tiny`, `corridor`, `no_path`, `open_arena_20x20`, `maze_20x20`, `weighted_small`).
- test: 38 Catch2 v3 test cases — `Grid`, `GridLoader`, all three heuristics (with property generators), all three algorithms, cross-algorithm equivalence (`A*(h≡0) == Dijkstra`), CLI parser edge cases, registry, and visualizer rendering.

### Changed
- docs(copilot): verification table in `.github/copilot-instructions.md` now uses `cmake --preset` commands and `cmake -S pae` for the long form, matching the actual CI. Adds a callout that `-S .` is rejected because of the nested-directory layout. Stops new contributors and Copilot from regenerating the old, broken commands.
- ci: every workflow (`ci.yml`, `validate.yml`, `release.yml`) now configures with `cmake -S pae -B pae/build*`, matching the README and the local scripts. Fixes the latent path mismatch where `release.yml` expected `pae/build-rel/pae` but the previous `-S .` configure produced `pae/build-rel/pae/pae`.
- build(metrics): `pae_metrics` is now a real STATIC library (was INTERFACE) so it can carry the new `Rss.cpp` translation unit. The dependency cycle break (split into `pae_metrics` + `pae_benchmark`) is preserved.
- feat(cli, bench): the `--benchmark` sweep now iterates `{manhattan, euclidean, chebyshev, octile}` (was 3 heuristics → 4). `--help` reflects the new heuristic.
- build(metrics): split `pae_metrics` into a header-only INTERFACE library (struct only) and a new `pae_benchmark` static library (orchestration), eliminating the latent `pae_metrics ↔ pae_algorithms` dependency cycle.
- build(headers): single `pae_headers` interface library now holds the public include path; sub-CMakeLists no longer depend on `${CMAKE_SOURCE_DIR}` so the project builds correctly under either `cmake -S . -B build` or `cmake -S pae -B build`.
- build(scripts): `run-checks.sh` and `run-benchmarks.sh` now build via `cmake -S pae -B pae/build*` for path consistency.

### Deprecated
- _none_

### Removed
- _none_

### Fixed
- fix(io): `GridLoader` no longer interprets `#`-prefixed body rows as comments, so full obstacle rows like `#####` parse correctly. The `#` comment marker is honoured only **before** the dimensions header.
- fix(algo): `AStar` lazy-stale check now compares `gCost` (cheap, exact) instead of recomputing `f = g + h`; eliminates a redundant heuristic call per pop.
- fix(factory): `Registry::reg` uses `operator[] = …` instead of `emplace`, so a second `registerAll()` call (e.g. between Catch2 test cases) overwrites stale closures.
- fix(tests): added missing `<algorithm>`, `<vector>`, `<initializer_list>`, `<string>`, and `pae/metrics/Metrics.hpp` includes to test files; renamed CLI tests to avoid leading `--` (which Catch2's argument parser otherwise consumed when ctest invoked the binary).

### Security
- security: documented threat model in `SECURITY.md`; pinned Catch2 dependency in `FetchContent`.

### Verified locally on Apple clang 21
- **44 / 44** Catch2 tests pass in Debug (was 38; +4 octile, +2 visualizer snapshot).
- **44 / 44** Catch2 tests pass under `-fsanitize=address,undefined`, no findings.
- **47 / 47** Catch2 tests pass with `cmake --preset perf` (44 regular + 3 perf-budget); `ctest -L perf` runs only the budget cases and they finish in ≈ 20 ms total.
- `pae --benchmark` on `maze_20x20.txt` produces a stable comparison: A*+Manhattan = 146 expanded, A*+Octile = 157, A*+Euclidean = 157, A*+Chebyshev = 167, BFS = 198, Dijkstra = 200 — all converging on the same optimal path (length 59, cost 58).
- `pae --benchmark` on the new `open_arena_50x50.txt` shows the heuristic ranking textbook-correctly under load: A*+Manhattan = 992 expanded, A*+Octile = 1371, A*+Euclidean = 1425, A*+Chebyshev = 1447, BFS / Dijkstra = 1726.
- `pae` correctly demonstrates BFS-vs-weighted divergence on `weighted_small.txt` (BFS picks 6-step heavy path; Dijkstra/A* pick 8-step weight-aware path).
- Built with `-DPAE_TRUE_RSS=ON`, the CLI prints `rss_delta_B: 32768` on `maze_20x20.txt` runs and the JSON / CSV benchmark reports gain a `rss_delta_bytes` column.

---

## How releases work

When a tag is cut:

1. The maintainer runs `pae/scripts/bump-version.sh <new>` which
   updates the version line in `pae/CMakeLists.txt` and renames the
   `[Unreleased]` heading in this file to `[<new>] - YYYY-MM-DD`.
2. A fresh `[Unreleased]` block is added at the top.
3. Commit message: `chore(release): v<new>`.
4. `git tag v<new>` and `git push --tags`.
5. The `release.yml` workflow attaches Linux/macOS/Windows binaries to
   the GitHub Release.

Planned releases are listed in [`ROADMAP.md`](ROADMAP.md). Per-version
content commitments are in [`IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md) §6.

---

## Past releases

_(none yet — first tag will be `v0.1.0`)_
