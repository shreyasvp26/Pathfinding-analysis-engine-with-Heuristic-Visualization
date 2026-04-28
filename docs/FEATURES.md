# Features — Pathfinding Analysis Engine

The canonical state of every feature.
Format: one row per feature, columns: ID, Title, Phase, Owner, Status, Notes.

When status changes, update this row in the same PR. Status values:
`Pending` → `In Progress` → `Completed` → (rare) `Cancelled`.

The `[Unreleased]` block in [`CHANGELOG.md`](CHANGELOG.md) holds the
matching change-log entry; when we tag a release, that block is moved
under the new version heading.

---

## V0 — Foundations

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-001 | Top-level repo skeleton (README, AGENTS, SECURITY, LICENSE, .gitignore, .gitattributes, .editorconfig) | `@build` | Completed | initial scaffold |
| F-002 | `.clang-format` + `.clang-tidy` profiles | `@build` | Completed | LLVM-derived |
| F-003 | `.pre-commit-config.yaml` with clang-format, secrets, large-file checks | `@build` | Completed | |
| F-004 | Top-level `CMakeLists.txt` delegating to `pae/CMakeLists.txt` | `@build` | Completed | |
| F-005 | `pae/CMakeLists.txt` builds the engine + binary; Catch2 via FetchContent | `@build` | Completed | dep DAG with `pae_headers` interface; split `pae_metrics` (header) ↔ `pae_benchmark` (impl) to break cycle |
| F-006 | `tests/test_smoke.cpp` passes via CTest | `@qa` | Completed | one of 38 |
| F-007 | CI matrix (Linux, macOS, Windows) green on empty repo | `@build` | Pending | scaffolded in `.github/workflows/ci.yml`; needs first push |

## V1 — Vertical slice MVP

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-101 | `pae::core::Coord`, `Cell`, `Node`, `Grid` (basic ctor, `inBounds`, `at`, `toIndex`, `neighbors4`) | `@core` | Completed | LLD §2 |
| F-102 | `pae::io::GridLoader::loadFromFile` + `loadFromString` | `@core` | Completed | LLD §2.5; supports comments + weighted cells |
| F-103 | `pae::heur::IHeuristic` interface | `@heuristic` | Completed | LLD §3.1 |
| F-104 | `pae::heur::Manhattan` concrete | `@heuristic` | Completed | LLD §3.2 |
| F-105 | `pae::algo::IPathfinder` + `RunConfig` + `Result` | `@algorithm` | Completed | LLD §4.1 |
| F-106 | `pae::algo::BFS` concrete | `@algorithm` | Completed | LLD §4.4 |
| F-107 | `pae::viz::IVisualizer` interface | `@viz` | Completed | LLD §5.1 (5 hooks: start/enqueue/expand/path/complete) |
| F-108 | `pae::viz::NullVisualizer` (no-op) | `@viz` | Completed | LLD §5.3 |
| F-109 | `pae::viz::CliVisualizer` Final mode (final path only) | `@viz` | Completed | LLD §5.2 |
| F-110 | `pae::metrics::Metrics` (counters + wall time) | `@perf` | Completed | LLD §6.1; includes `approxPeakBytes` model |
| F-111 | `pae::factory::Registry<T>` + `registerAll()` | `@core` | Completed | LLD §7; idempotent re-registration |
| F-112 | `pae::cli::parseArgs` + `pae::App::run` + `main.cpp` | `@core` | Completed | LLD §8; full exit-code matrix |
| F-113 | `tests/test_grid.cpp`, `test_grid_loader.cpp`, `test_manhattan.cpp`, `test_bfs.cpp` | `@qa` | Completed | all green |
| F-114 | Sample maps `corridor.txt`, `tiny.txt`, `no_path.txt`, `maze_20x20.txt` | `@core` | Completed | also `open_arena_20x20.txt`, `weighted_small.txt` |

## V2 — Algorithm trio

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-201 | Per-cell weights in `Grid` | `@core` | Completed | `weight()`; loader recognises `0`–`9` digits |
| F-202 | `pae::algo::Dijkstra` | `@algorithm` | Completed | LLD §4.3; lazy decrease-key |
| F-203 | `pae::algo::AStar` taking `IHeuristic&` | `@algorithm` | Completed | LLD §4.2; staleness check on `gCost` |
| F-204 | `pae::heur::Euclidean` | `@heuristic` | Completed | LLD §3.2 |
| F-205 | `pae::viz::CliVisualizer::Step` mode + fps cap | `@viz` | Completed | ANSI clear + 30 fps default cap |
| F-206 | Cross-algorithm equivalence test (A\* with `h≡0` ≡ Dijkstra) | `@qa` | Completed | `test_cross_algorithm.cpp` (3 fixtures) |
| F-207 | `tests/test_astar.cpp`, `test_dijkstra.cpp`, `test_euclidean.cpp` | `@qa` | Completed | |
| F-208 | `pae --benchmark` mode prints comparison table | `@perf` | Completed | with median + p95 |
| F-209 | Additional sample maps | `@core` | Completed | `maze_50x50.txt` (recursive-backtracker, seed 42) and `open_arena_50x50.txt` (density 0.30, seed 1) committed; reproducible via `pae/scripts/gen_maze.py` |

## V3 — Robustness

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-301 | `pae::heur::Chebyshev` | `@heuristic` | Completed | LLD §3.2 |
| F-302 | 8-connectivity (`Grid::neighbors8`, `--diagonal` flag) | `@core` + `@algorithm` | Completed | both algorithms branch on `cfg.diagonal` |
| F-303 | Property tests for heuristic invariants | `@qa` | Completed | non-negativity, symmetry, identity (4096+ cases per heuristic) |
| F-304 | ASan + UBSan jobs in CI | `@qa` + `@build` | Pending | locally verified clean; CI wiring is in `ci.yml` |
| F-305 | Edge-case maps (dense obstacle, isolated start, full-blocked goal) | `@qa` | Completed | `no_path.txt` covers full-row blocker |
| F-306 | `--diagonal` + Manhattan combo guard | `@core` | Completed | App refuses with explanatory error and exit 1 |

## V4 — Benchmark hardening

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-401 | `Benchmark::sweep` with warmup + median + p95 | `@perf` | Completed | `Config{warmup=3, repetitions=30}` defaults |
| F-402 | `Report::writeCsv`, `writeJson` | `@perf` | Completed | `pae_bench` accepts `--csv`, `--json` |
| F-403 | `bench_pathfinders.cpp` runs against every map in `pae/maps/` | `@perf` | Completed | via `scripts/run-benchmarks.sh` |
| F-404 | NFR-2 perf budget tests in CI | `@perf` + `@build` | Pending | `PAE_PERF_BUDGET` option scaffolded |
| F-405 | Baselines committed in `pae/benchmarks/baselines/` | `@perf` | Pending | will be populated on first CI run |
| F-406 | CI commenter: posts perf delta vs main on PRs | `@build` | Pending | needs `auto-ticket.yml` extension |

## V5 — Release polish

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-501 | `--help`, `--version`, exit-code matrix matches REQUIREMENTS.md §FR-6 | `@core` | Completed | exit 0 success, 1 user-error / not-found, 2 engine-error |
| F-502 | `docs/CHANGELOG.md` `[1.0.0]` entry | all | Pending | block currently in `[Unreleased]` |
| F-503 | Tag `v1.0.0`; release workflow attaches Linux/macOS/Windows binaries | `@build` | Pending | |
| F-504 | README quick-start verified by fresh-clone CI job | `@build` | Pending | |

---

## Post-1.0 — from `EXTENSIONS.md` and `ROADMAP.md`

These rows enter `In Progress` only after v1.0 is tagged.

| ID | Title | Owner | Target | Status |
|----|-------|-------|--------|--------|
| F-601 | `Octile` heuristic | `@heuristic` | v0.2 | Completed | LLD §3.2 + new test cases; tight admissible heuristic for 8-conn-with-diag-√2 |
| F-602 | Random maze generator | `@core` | v0.2 | Completed | `pae/scripts/gen_maze.py` (Python, deterministic, two modes) |
| F-603 | Bidirectional A\* | `@algorithm` | v1.2 | Pending |
| F-604 | Jump Point Search | `@algorithm` | v1.3 | Pending |
| F-605 | Theta\* (any-angle) | `@algorithm` | v1.4 | Pending |
| F-606 | True RSS metric (`getrusage`) | `@perf` | v0.2 | Completed | `pae::metrics::maxResidentBytes` gated on `PAE_TRUE_RSS`; surfaces in CLI summary, JSON, CSV |
| F-607 | Optional Raylib GUI visualizer | `@viz` | v1.6 | Pending |
| F-608 | Logging framework (`pae::log`) | `@core` | v1.7 | Pending |
| F-609 | JSON map format alternative | `@core` | v1.7 | Pending |
| F-610 | `CMakePresets.json` for one-command builds | `@build` | v0.2 | Completed | 5 configure presets + 3 workflow presets, requires CMake ≥ 3.25 |
| F-611 | Mermaid architecture / class / sequence diagrams | `@build` + all | v0.2 | Completed | `design/diagrams.md` + 4 `.mmd` source files |
| F-612 | "How to add a new algorithm" tutorial | `@core` | v0.2 | Completed | `docs/TUTORIAL.md`; uses GBFS as the running example |
| F-613 | Snapshot tests for `CliVisualizer` | `@viz` + `@qa` | v0.2 | Completed | corridor + obstacle maps; locks glyphs and row separators |

---

## Architecture reference

For the architectural meaning of each feature, see:

- `F-1xx` → [`LLD.md`](LLD.md) §2–§8
- `F-2xx` → [`ALGORITHMS.md`](ALGORITHMS.md), [`HEURISTICS.md`](HEURISTICS.md)
- `F-3xx` → [`TESTING.md`](TESTING.md), [`REQUIREMENTS.md`](REQUIREMENTS.md) §NFR-1
- `F-4xx` → [`PERFORMANCE.md`](PERFORMANCE.md)
- `F-5xx` → [`IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md) §V5
- `F-6xx` → [`EXTENSIONS.md`](EXTENSIONS.md), [`ROADMAP.md`](ROADMAP.md)
