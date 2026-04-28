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
| F-001 | Top-level repo skeleton (README, AGENTS, SECURITY, LICENSE, .gitignore, .gitattributes, .editorconfig) | `@build` | Completed | this PR / initial scaffold |
| F-002 | `.clang-format` + `.clang-tidy` profiles | `@build` | Completed | LLVM-derived |
| F-003 | `.pre-commit-config.yaml` with clang-format, secrets, large-file checks | `@build` | Completed | |
| F-004 | Top-level `CMakeLists.txt` delegating to `pae/CMakeLists.txt` | `@build` | Completed | |
| F-005 | `pae/CMakeLists.txt` builds an empty `pae` binary | `@build` | Pending | wires Catch2 via FetchContent |
| F-006 | `tests/test_smoke.cpp` passes via CTest | `@qa` | Pending | proves the build/test wiring |
| F-007 | CI matrix (Linux, macOS, Windows) green on empty repo | `@build` | Pending | |

## V1 — Vertical slice MVP

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-101 | `pae::core::Coord`, `Cell`, `Node`, `Grid` (basic ctor, `inBounds`, `at`, `toIndex`, `neighbors4`) | `@core` | Pending | LLD §2 |
| F-102 | `pae::io::GridLoader::loadFromFile` + `loadFromString` | `@core` | Pending | LLD §2.5 |
| F-103 | `pae::heur::IHeuristic` interface | `@heuristic` | Pending | LLD §3.1 |
| F-104 | `pae::heur::Manhattan` concrete | `@heuristic` | Pending | LLD §3.2 |
| F-105 | `pae::algo::IPathfinder` + `RunConfig` + `Result` | `@algorithm` | Pending | LLD §4.1 |
| F-106 | `pae::algo::BFS` concrete | `@algorithm` | Pending | LLD §4.4 |
| F-107 | `pae::viz::IVisualizer` interface | `@viz` | Pending | LLD §5.1 |
| F-108 | `pae::viz::NullVisualizer` (no-op) | `@viz` | Pending | LLD §5.3 |
| F-109 | `pae::viz::CliVisualizer` Final mode (final path only) | `@viz` | Pending | LLD §5.2 |
| F-110 | `pae::metrics::Metrics` (counters + wall time) | `@perf` | Pending | LLD §6.1 |
| F-111 | `pae::factory::Registry<T>` + `registerAll()` | `@core` | Pending | LLD §7 |
| F-112 | `pae::cli::parseArgs` + `pae::App::run` + `main.cpp` | `@core` | Pending | LLD §8 |
| F-113 | `tests/test_grid.cpp`, `test_grid_loader.cpp`, `test_manhattan.cpp`, `test_bfs.cpp` | `@qa` | Pending | TESTING.md §3 |
| F-114 | Sample maps `corridor.txt`, `tiny.txt`, `no_path.txt`, `maze_20x20.txt` | `@core` | Pending | |

## V2 — Algorithm trio

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-201 | Per-cell weights in `Grid` | `@core` | Pending | Loader extension |
| F-202 | `pae::algo::Dijkstra` | `@algorithm` | Pending | LLD §4.3 |
| F-203 | `pae::algo::AStar` taking `IHeuristic&` | `@algorithm` | Pending | LLD §4.2 |
| F-204 | `pae::heur::Euclidean` | `@heuristic` | Pending | LLD §3.2 |
| F-205 | `pae::viz::CliVisualizer::Step` mode + fps cap | `@viz` | Pending | LLD §5.2 |
| F-206 | Cross-algorithm equivalence test (A\* with `h≡0` ≡ Dijkstra) | `@qa` | Pending | ALGORITHMS.md §4.6 |
| F-207 | `tests/test_astar.cpp`, `test_dijkstra.cpp`, `test_euclidean.cpp` | `@qa` | Pending | |
| F-208 | `pae --benchmark` mode prints comparison table | `@perf` | Pending | PERFORMANCE.md §3 |
| F-209 | Sample maps `weighted_small.txt`, `open_arena_50x50.txt`, `maze_50x50.txt` | `@core` | Pending | |

## V3 — Robustness

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-301 | `pae::heur::Chebyshev` | `@heuristic` | Pending | LLD §3.2 |
| F-302 | 8-connectivity (`Grid::neighbors8`, `--diagonal` flag) | `@core` + `@algorithm` | Pending | LLD §2.4 |
| F-303 | Property tests for heuristic invariants | `@qa` | Pending | TESTING.md §4.1 |
| F-304 | ASan + UBSan jobs in CI | `@qa` + `@build` | Pending | TESTING.md §6 |
| F-305 | Edge-case maps (dense obstacle, isolated start, full-blocked goal) | `@qa` | Pending | TESTING.md §3 |
| F-306 | `--diagonal` + Manhattan combo guard (rejected unless `--allow-inadmissible`) | `@core` | Pending | EC-14 in TESTING.md |

## V4 — Benchmark hardening

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-401 | `Benchmark::sweep` with warmup + median + p95 | `@perf` | Pending | PERFORMANCE.md §3 |
| F-402 | `Report::writeCsv`, `writeJson` | `@perf` | Pending | PERFORMANCE.md §4 |
| F-403 | `bench_pathfinders.cpp` runs against every map in `pae/maps/` | `@perf` | Pending | |
| F-404 | NFR-2 perf budget tests in CI | `@perf` + `@build` | Pending | REQUIREMENTS.md §NFR-2 |
| F-405 | Baselines committed in `pae/benchmarks/baselines/` | `@perf` | Pending | PERFORMANCE.md §9 |
| F-406 | CI commenter: posts perf delta vs main on PRs | `@build` | Pending | |

## V5 — Release polish

| ID | Title | Owner | Status | Notes |
|----|-------|-------|--------|-------|
| F-501 | `--help`, `--version`, exit-code matrix matches REQUIREMENTS.md §FR-6 | `@core` | Pending | |
| F-502 | `docs/CHANGELOG.md` `[1.0.0]` entry | all | Pending | |
| F-503 | Tag `v1.0.0`; release workflow attaches Linux/macOS/Windows binaries | `@build` | Pending | |
| F-504 | README quick-start verified by fresh-clone CI job | `@build` | Pending | |

---

## Post-1.0 — from `EXTENSIONS.md` and `ROADMAP.md`

These rows enter `In Progress` only after v1.0 is tagged.

| ID | Title | Owner | Target | Status |
|----|-------|-------|--------|--------|
| F-601 | `Octile` heuristic | `@heuristic` | v1.1 | Pending |
| F-602 | Random maze generator (`pae --gen maze ...`) | `@core` | v1.1 | Pending |
| F-603 | Bidirectional A\* | `@algorithm` | v1.2 | Pending |
| F-604 | Jump Point Search | `@algorithm` | v1.3 | Pending |
| F-605 | Theta\* (any-angle) | `@algorithm` | v1.4 | Pending |
| F-606 | True RSS metric (`getrusage`) | `@perf` | v1.5 | Pending |
| F-607 | Optional Raylib GUI visualizer | `@viz` | v1.6 | Pending |
| F-608 | Logging framework (`pae::log`) | `@core` | v1.7 | Pending |
| F-609 | JSON map format alternative | `@core` | v1.7 | Pending |

---

## Architecture reference

For the architectural meaning of each feature, see:

- `F-1xx` → [`LLD.md`](LLD.md) §2–§8
- `F-2xx` → [`ALGORITHMS.md`](ALGORITHMS.md), [`HEURISTICS.md`](HEURISTICS.md)
- `F-3xx` → [`TESTING.md`](TESTING.md), [`REQUIREMENTS.md`](REQUIREMENTS.md) §NFR-1
- `F-4xx` → [`PERFORMANCE.md`](PERFORMANCE.md)
- `F-5xx` → [`IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md) §V5
- `F-6xx` → [`EXTENSIONS.md`](EXTENSIONS.md), [`ROADMAP.md`](ROADMAP.md)
