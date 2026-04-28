# Requirements — Pathfinding Analysis Engine

Source of truth for what this project must do and how we know it does it.
Every line below is testable. If a requirement here changes, the
matching row in [`FEATURES.md`](FEATURES.md), the test in
[`TESTING.md`](TESTING.md), and the PR description must change with it.

---

## 1. Functional requirements (FR)

### FR-1 — Algorithms

| ID | Requirement | Verification |
|----|-------------|-------------|
| FR-1.1 | Implement A\* search with pluggable heuristic. | `tests/test_astar.cpp` — finds known optimal path on 3 reference maps. |
| FR-1.2 | Implement Dijkstra's algorithm. | `tests/test_dijkstra.cpp` — total cost equals the manually computed shortest path on a weighted reference map. |
| FR-1.3 | Implement Breadth-First Search. | `tests/test_bfs.cpp` — returns the path with the **minimum number of steps** on an unweighted map. |
| FR-1.4 | All algorithms expose the same interface (`IPathfinder`). | Compile-time: `static_assert(std::is_base_of_v<IPathfinder, T>)` in the registry. Runtime: `Registry::create("astar"|"dijkstra"|"bfs")` returns `unique_ptr<IPathfinder>`. |
| FR-1.5 | Algorithms are interchangeable from the CLI without recompilation. | `--algo {astar,dijkstra,bfs}` switch works on every map. |
| FR-1.6 | Each algorithm reports the same `Metrics` schema. | All implementations write to `Metrics` — verified by parametric Catch2 test running against every algorithm. |

### FR-2 — Heuristics

| ID | Requirement | Verification |
|----|-------------|-------------|
| FR-2.1 | Implement Manhattan distance. | Property test: `h(a,b) == h(b,a)`, `h(a,a) == 0`, `h(a,b) <= manhattan_actual(a,b)` for all 4-conn paths. |
| FR-2.2 | Implement Euclidean distance. | Same property tests; floating-point comparison within 1e-9. |
| FR-2.3 | Implement Chebyshev distance (optional but in-scope for v0.2). | Property test: `h(a,b) = max(|dx|,|dy|)`. |
| FR-2.4 | A\* accepts any class implementing `IHeuristic`. | Compile-time interface check + runtime injection via constructor. |
| FR-2.5 | A\* with a zero heuristic produces the same path cost as Dijkstra. | Cross-algorithm equivalence test on a uniform-weight map. |

### FR-3 — Grid system

| ID | Requirement | Verification |
|----|-------------|-------------|
| FR-3.1 | Support 2D grids of arbitrary size up to 1024×1024. | Load test on a 1024×1024 random maze; succeeds in <2 s on dev machine. |
| FR-3.2 | Each cell may be **walkable**, **obstacle**, **start**, or **end**. | `Cell` enum + `GridLoader` parses `.`, `#`, `S`, `E`. |
| FR-3.3 | Optional weighted cells (per-cell move cost ≥ 1). | `Grid::weight(coord)` returns `int64_t`; default 1. |
| FR-3.4 | Exactly one start and one end node per grid (validated on load). | `GridLoader::validate()` throws if violated. |
| FR-3.5 | The grid is immutable during a single algorithm run. | `IPathfinder::run(const Grid&, …)` — `const&` enforces. |

### FR-4 — Visualization

| ID | Requirement | Verification |
|----|-------------|-------------|
| FR-4.1 | Provide step-by-step visualization of traversal (not only the final path). | CLI flag `--visualize step` shows ANSI frames at each expansion. |
| FR-4.2 | Distinguish: obstacle, start, end, frontier (open set), visited (closed set), final path. | Each maps to a unique character + colour (or character only with `--no-color`). |
| FR-4.3 | Final-path-only mode (`--visualize final`) shows just the result. | Snapshot test. |
| FR-4.4 | Headless mode (`--visualize none`) emits no rendering — only metrics. | Verified by missing-output assertion in tests. |
| FR-4.5 | Visualization is decoupled from algorithms (algorithms emit events, not characters). | `IVisualizer::onExpand`, `onEnqueue`, `onPathFound` API. |

### FR-5 — Performance / metrics

| ID | Requirement | Verification |
|----|-------------|-------------|
| FR-5.1 | Track nodes expanded, nodes enqueued, path length, path cost. | `Metrics` struct populated by every algorithm; asserted in tests. |
| FR-5.2 | Track wall-clock execution time using `std::chrono::steady_clock`. | Reported in microseconds. |
| FR-5.3 | Track approximate peak memory of working set (open + closed sets, parent map). | Computed analytically (`sizeof(Node) * count`); not OS-level RSS. |
| FR-5.4 | `--benchmark` mode runs all algorithms on the same map and prints a comparison table. | CLI integration test against `maps/maze_20x20.txt`. |
| FR-5.5 | Benchmark results are deterministic and reproducible (±2% over 30 runs). | CI job runs the same benchmark twice and asserts. |

### FR-6 — CLI

| ID | Requirement | Verification |
|----|-------------|-------------|
| FR-6.1 | Single binary: `pae`. | `add_executable(pae …)` in `pae/CMakeLists.txt`. |
| FR-6.2 | Required flags: `--map <path>`, optional `--algo`, `--heuristic`, `--visualize`, `--benchmark`, `--diagonal`, `--seed`, `--no-color`. | Parsed by `pae::cli::parseArgs`; unit-tested. |
| FR-6.3 | `pae --help` prints usage; `pae --version` prints SemVer. | Snapshot tests. |
| FR-6.4 | Non-zero exit code on: invalid map, no path found, unknown algo / heuristic, file IO errors. | Exit-code matrix in `TESTING.md` §3. |

---

## 2. Non-functional requirements (NFR)

### NFR-1 — Code quality

| ID | Requirement | Threshold | How verified |
|----|------------|----------|--------------|
| NFR-1.1 | C++17, no compiler-specific extensions. | — | CI matrix builds on GCC 11+, Clang 14+, MSVC 19.30+. |
| NFR-1.2 | Compile with `-Wall -Wextra -Werror -Wpedantic` (or MSVC `/W4 /WX`). | Zero warnings. | CI. |
| NFR-1.3 | clang-tidy with [`.clang-tidy`](../.clang-tidy) profile. | Zero new errors. | CI. |
| NFR-1.4 | clang-format with [`.clang-format`](../.clang-format) profile. | `--Werror` clean. | Pre-commit + CI. |
| NFR-1.5 | No memory leaks under ASan. | Zero. | CI Debug job. |
| NFR-1.6 | No undefined behaviour under UBSan. | Zero. | CI Debug job. |

### NFR-2 — Performance

| ID | Requirement | Threshold | Map |
|----|------------|----------|-----|
| NFR-2.1 | A\* with Manhattan finishes a 100×100 maze in < 50 ms (Release, dev machine ≥ 2 GHz). | < 50 ms median over 30 runs. | `maps/maze_100x100.txt` |
| NFR-2.2 | Dijkstra finishes a 100×100 weighted map in < 200 ms. | < 200 ms median. | `maps/weighted_100x100.txt` |
| NFR-2.3 | BFS finishes a 100×100 unweighted map in < 100 ms. | < 100 ms median. | `maps/maze_100x100.txt` |
| NFR-2.4 | Memory consumption ≤ ~5 MB for a 1024×1024 grid. | Per-cell footprint < 5 bytes. | Computed analytically. |

### NFR-3 — Testability

| ID | Requirement |
|----|------------|
| NFR-3.1 | Every public method on every interface (`IPathfinder`, `IHeuristic`, `IVisualizer`) has at least one unit test. |
| NFR-3.2 | Test suite runs end-to-end in < 10 s on Release, < 30 s on Debug+ASan+UBSan. |
| NFR-3.3 | Property tests use deterministic seeds; flaky tests are immediately quarantined as bugs. |

### NFR-4 — Documentation

| ID | Requirement |
|----|------------|
| NFR-4.1 | Every public header has a top-of-file `///` summary describing intent, ownership, and thread-safety. |
| NFR-4.2 | Every algorithm has a `docs/ALGORITHMS.md` section with pseudo-code, complexity, optimality conditions. |
| NFR-4.3 | Every PR updates `CHANGELOG.md` (under `[Unreleased]`). |

### NFR-5 — Portability

| Target | Status |
|--------|--------|
| Linux x86_64 (Ubuntu 22.04+) | first-class, CI-tested |
| macOS arm64 (Apple Silicon) | first-class, CI-tested |
| Windows x86_64 (MSVC 2022+) | first-class, CI-tested |
| Linux aarch64 | best-effort |
| FreeBSD | not supported |

---

## 3. Constraints

| ID | Constraint |
|----|-----------|
| C-1 | Implementable by a single student in **6–12 hours** for the V1 vertical slice (FR-1, FR-2, FR-3, FR-4 step mode, FR-5.1, FR-5.2). The remainder is sized as v0.2 and v0.3 in [`ROADMAP.md`](ROADMAP.md). |
| C-2 | No databases, no networking, no GUI framework. The product is **one binary**. |
| C-3 | The only external runtime dependency is the C++17 standard library. Test framework is fetched at configure time. |
| C-4 | Code MUST demonstrate **abstraction** (interfaces), **inheritance** (concrete classes derive from interfaces), **polymorphism** (algorithms used through interfaces, not concrete types), and **separation of concerns** (CLI / algorithm / heuristic / visualization / metrics live in distinct translation units and folders). |
| C-5 | No `using namespace std;` in headers. Avoid `using namespace` in `.cpp` files except inside narrow function scope. |
| C-6 | All public APIs use `std::filesystem::path` for paths, `std::string_view` for read-only strings, owning containers (`std::vector`, `std::string`) for owned data. |

---

## 4. Out of scope (v1)

- 3D grids
- Hex grids
- Real-time replanning (D\*, LPA\*, ARA\*)
- GPU-accelerated traversal
- Network multiplayer / distributed pathfinding
- ML-learned heuristics
- Curses-based or web-based UI
- Internationalisation

These are tracked in [`EXTENSIONS.md`](EXTENSIONS.md) for future
consideration; none of them is a v1 commitment.

---

## 5. Acceptance — "Done" definition for v1.0

The project is v1.0 when **all** of the following are true:

1. Every `[ ]` in this document's tables is `[x]` (or has a justified
   waiver in [`BUGS.md`](BUGS.md)).
2. CI is green on Linux, macOS, Windows.
3. `ctest` reports **0 failures, 0 skipped** (Debug + Release).
4. ASan + UBSan show **0 issues**.
5. clang-tidy + clang-format are clean.
6. Benchmark report (`pae --benchmark --map maps/maze_100x100.txt`)
   produces stable numbers (±2% across 30 runs).
7. `docs/CHANGELOG.md` has a `[1.0.0]` heading.
8. The repo is tagged `v1.0.0`.
