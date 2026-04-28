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
- 38 / 38 Catch2 tests pass in Debug.
- 38 / 38 Catch2 tests pass under `-fsanitize=address,undefined`.
- `pae --benchmark` on `maze_20x20.txt` produces a stable comparison: A*+Manhattan = 146 expanded, A*+Euclidean = 157, A*+Chebyshev = 167, BFS = 198, Dijkstra = 200 — all converging on the same optimal path (length 59, cost 58).
- `pae` correctly demonstrates BFS-vs-weighted divergence on `weighted_small.txt` (BFS picks 6-step heavy path; Dijkstra/A* pick 8-step weight-aware path).

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
