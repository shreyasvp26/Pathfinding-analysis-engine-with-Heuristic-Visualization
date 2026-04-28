# Testing Strategy — Pathfinding Analysis Engine

We treat tests as **the executable definition of correctness**. Every
public method of every interface in [`LLD.md`](LLD.md) has at least
one test. Every requirement in [`REQUIREMENTS.md`](REQUIREMENTS.md)
maps to one or more tests. CI fails the PR if either is violated.

Test framework: **Catch2 v3** (fetched via `FetchContent`).

---

## 1. Test taxonomy

| Tier | What it verifies | Examples | Where |
|------|------------------|----------|-------|
| Unit | One class, one method, one behaviour | `Grid::inBounds`, `Manhattan::estimate(a,a) == 0` | `pae/tests/test_<thing>.cpp` |
| Property | Invariants over many random inputs | "heuristic is symmetric for 1000 random coord pairs" | `pae/tests/test_<thing>.cpp` (mixed in) |
| Integration | Two or more components together | A\* + Manhattan + Grid + GridLoader | `pae/tests/test_astar.cpp`, `test_dijkstra.cpp` |
| Cross-algorithm | Two algorithms produce equivalent results | A\* with `h≡0` ≡ Dijkstra | `pae/tests/test_cross_algorithm.cpp` |
| End-to-end | The `pae` binary on a real map | `pae --map fixtures/corridor.txt --algo bfs` | `pae/tests/test_cli.cpp` |
| Snapshot | Visualizer output matches a stored expected | `CliVisualizer::Final` on `tiny.txt` | `pae/tests/test_visualizer.cpp` + `fixtures/snapshots/` |
| Sanitizer-only | Detects UB / leaks under ASan/UBSan | full Debug pass | gated by CI Debug job |
| Performance | Run time ≤ NFR-2 budget | A\* 100×100 < 50 ms | `pae/tests/test_perf_budget.cpp` (CI runner only) |

---

## 2. Folder layout

```
pae/tests/
├── CMakeLists.txt
├── helpers/
│   ├── TestGrid.hpp          — small builders for fixture grids
│   └── ZeroHeuristic.hpp     — `IHeuristic` returning 0; for cross-algo tests
├── fixtures/
│   ├── tiny.txt              — 3×3, S adjacent to E
│   ├── corridor.txt          — 1×N corridor, S on left, E on right
│   ├── no_path.txt           — wall separating S from E
│   ├── dense_obstacle.txt    — only one valid detour
│   ├── weighted_small.txt    — short heavy vs. long light path
│   └── snapshots/
│       ├── tiny.bfs.final.txt
│       ├── corridor.astar.manhattan.step_5.txt
│       └── …
├── test_grid.cpp
├── test_grid_loader.cpp
├── test_manhattan.cpp
├── test_euclidean.cpp
├── test_chebyshev.cpp
├── test_bfs.cpp
├── test_dijkstra.cpp
├── test_astar.cpp
├── test_cross_algorithm.cpp
├── test_visualizer.cpp
├── test_metrics.cpp
├── test_registry.cpp
├── test_cli.cpp
└── test_perf_budget.cpp      — only enabled when -DPAE_PERF_BUDGET=ON
```

CTest labels (`add_test(... PROPERTIES LABELS "...")`) split tests into
runnable groups:

```bash
ctest --test-dir build --output-on-failure                       # all
ctest --test-dir build --output-on-failure -L core               # core only
ctest --test-dir build --output-on-failure -L "algorithm|heuristic"
ctest --test-dir build --output-on-failure -L cross              # equivalence tests
ctest --test-dir build --output-on-failure -L perf               # perf budgets (CI runner)
```

---

## 3. Edge cases — must all be tested

(These come from [`REQUIREMENTS.md`](REQUIREMENTS.md) §3 and
[`ALGORITHMS.md`](ALGORITHMS.md) §7. Every algorithm's test file
exercises each row.)

| ID | Scenario | Expected | Algorithm-specific notes |
|----|----------|----------|--------------------------|
| EC-01 | Start adjacent to End | `path = {S, E}`, cost = `weight(E)` | All algos. |
| EC-02 | Start == End | `path = {S}`, cost = 0 | All algos. Special-case at top of `run()`. |
| EC-03 | Start surrounded by obstacles | `Result{found=false}` | All algos. |
| EC-04 | End surrounded by obstacles | `Result{found=false}` | All algos. |
| EC-05 | No path through maze | `Result{found=false}` | All algos. |
| EC-06 | Single corridor | linear path | All algos. |
| EC-07 | Open arena | All algos optimal; A\* expands fewest with admissible h. | Verified by checking `metrics.nodesExpanded`. |
| EC-08 | Two paths: short heavy vs. long light | BFS picks short (it's shortest by step count); Dijkstra/A\* pick light. | Verifies BFS ≠ Dijkstra on weighted maps. |
| EC-09 | Grid 1024×1024 random maze | terminates < NFR-2 budget | `test_perf_budget.cpp`. |
| EC-10 | Malformed map (no `S`) | `GridLoader` throws `IoError` | Loader test. |
| EC-11 | Malformed map (multiple `S`) | `GridLoader` throws `IoError` | Loader test. |
| EC-12 | Invalid character in map | `GridLoader` throws `IoError` | Loader test. |
| EC-13 | Diagonal off + Manhattan + 8-conn-friendly map | A\* still optimal (Manhattan is admissible 4-conn). | Tests for `--diagonal` matrix. |
| EC-14 | Diagonal on + Manhattan | flagged inadmissible; A\* may pick non-optimal — we **document** but do not allow at the CLI without a `--allow-inadmissible` flag (default off). | CLI rejects this combo. |

---

## 4. Property tests

Catch2 supports `GENERATE` for property testing. We use deterministic
seeded ranges (no fully random behaviour in CI).

### 4.1 Heuristic invariants

```cpp
TEST_CASE("Manhattan is symmetric, non-negative, zero on diagonal",
          "[heuristic][property]") {
    Manhattan h;
    auto r1 = GENERATE(range(0, 64));
    auto c1 = GENERATE(range(0, 64));
    auto r2 = GENERATE(range(0, 64));
    auto c2 = GENERATE(range(0, 64));
    Coord a{r1, c1}, b{r2, c2};
    REQUIRE(h.estimate(a, b) >= 0.0);
    REQUIRE(h.estimate(a, b) == h.estimate(b, a));
    REQUIRE(h.estimate(a, a) == 0.0);
}
```

Each heuristic gets the same property battery (parametrised across
`Manhattan`, `Euclidean`, `Chebyshev`).

### 4.2 Algorithm invariants

```cpp
TEST_CASE("A* with zero heuristic finds same cost as Dijkstra",
          "[cross][algo]") {
    auto path = GENERATE(values<std::string>({
        "fixtures/tiny.txt",
        "fixtures/corridor.txt",
        "fixtures/dense_obstacle.txt",
        "fixtures/weighted_small.txt",
    }));
    Grid g = GridLoader::loadFromFile(path);
    ZeroHeuristic h0;
    AStar a{h0};
    Dijkstra d;
    auto ra = a.run(g, {}, nullptr, nullptr);
    auto rd = d.run(g, {}, nullptr, nullptr);
    REQUIRE(ra.found  == rd.found);
    REQUIRE(ra.totalCost == rd.totalCost);
}
```

### 4.3 BFS produces minimum-step path

```cpp
TEST_CASE("BFS path has minimum number of steps", "[bfs][property]") {
    auto path = GENERATE(values<std::string>({...}));
    Grid g = GridLoader::loadFromFile(path);
    BFS bfs;
    auto r = bfs.run(g, {}, nullptr, nullptr);
    if (r.found) {
        // Total cost should equal hop count (BFS treats every move as 1).
        REQUIRE(r.totalCost == static_cast<int64_t>(r.path.size()) - 1);
    }
}
```

---

## 5. Snapshot tests for the visualizer

We store a small set of expected ANSI-stripped frames in
`fixtures/snapshots/`:

```cpp
TEST_CASE("CliVisualizer Final mode on tiny.txt", "[viz]") {
    auto g = GridLoader::loadFromFile("fixtures/tiny.txt");
    BFS bfs;
    std::ostringstream oss;
    CliVisualizer viz{ {.mode = VizMode::Final, .color = false}, oss };
    auto r = bfs.run(g, {}, &viz, nullptr);
    REQUIRE(r.found);
    REQUIRE(oss.str() == readFile("fixtures/snapshots/tiny.bfs.final.txt"));
}
```

Snapshots are **`--no-color`** (ASCII-only) so diffs in PRs are
readable. A `--update-snapshots` build flag regenerates them
intentionally; CI rejects PRs that update snapshots without a matching
"Updates snapshot" line in the PR description.

---

## 6. Sanitizers

CMake exposes `-DPAE_SANITIZERS=ON` which turns on:

- `-fsanitize=address`
- `-fsanitize=undefined`
- `-fno-omit-frame-pointer`

`.github/workflows/ci.yml` runs a Debug-with-sanitizers job on
Ubuntu-latest. Any crash, leak, or UB report fails the build. Linux
is sufficient for catching almost everything; macOS and Windows are
covered by the regular Release builds.

We do **not** turn on ThreadSanitizer in v1 because the engine is
single-threaded. Once `Benchmark::sweep` becomes parallel (v0.4
stretch), TSan becomes a fourth sanitizer job.

---

## 7. Performance budget tests

`test_perf_budget.cpp` is opt-in via `-DPAE_PERF_BUDGET=ON`. It is run
in CI on a fixed runner type (`ubuntu-latest`, x86_64) with a warmup
and median-of-5 measurement, asserting:

- A\* + Manhattan on `maps/maze_100x100.txt` < 50 ms.
- Dijkstra on `maps/weighted_100x100.txt` < 200 ms.
- BFS on `maps/maze_100x100.txt` < 100 ms.

A failure here is treated as a regression and **must be** investigated
before merge — see [`PERFORMANCE.md`](PERFORMANCE.md) §4.

---

## 8. Determinism

Every test must be **deterministic** at the byte level.

- No `std::time(nullptr)` — use the `seed` field of `RunConfig`.
- No `std::random_device` — use `std::mt19937{seed}`.
- Tie-breaking in priority queues is fully specified (see
  `LLD.md` §2.3).
- Map files have stable line endings (LF) enforced by `.gitattributes`.
- Snapshot files are `--no-color`, no timestamps, no metrics inlined.

A flaky test is treated as a **bug** with a `B<id>` ticket; we do not
auto-retry.

---

## 9. Coverage targets (informative, not hard gate)

| Component | Target line coverage |
|-----------|----------------------|
| `pae_core` (Grid, Coord, etc.) | ≥ 95 % |
| `pae_algorithms` | ≥ 95 % |
| `pae_heuristics` | 100 % (they are 5-line functions) |
| `pae_io` | ≥ 90 % |
| `pae_visualization` | ≥ 75 % (rendering paths exercised by snapshots) |
| `pae_metrics` | ≥ 90 % |
| `pae_factory` | 100 % |
| `pae_cli` | ≥ 80 % (errors paths via integration tests) |

We collect coverage with `-fprofile-arcs -ftest-coverage` (gcc) or
`-fprofile-instr-generate -fcoverage-mapping` (clang) on the Debug
job and upload to GitHub Actions artefacts. We do not (yet) integrate
with Codecov — coverage is informative for v1.

---

## 10. Running tests locally

```bash
# Configure once
cmake -S . -B pae/build -DCMAKE_BUILD_TYPE=Debug -DPAE_SANITIZERS=ON
cmake --build pae/build -j

# Everything
ctest --test-dir pae/build --output-on-failure

# Subset
ctest --test-dir pae/build -L heuristic --output-on-failure

# Single test
ctest --test-dir pae/build -R "Manhattan is symmetric" --output-on-failure

# With CTest verbose
ctest --test-dir pae/build -V

# With Catch2 directly (more granular)
./pae/build/tests/pae_tests "[astar]" --reporter console
```

---

## 11. Adding a new test (checklist)

When adding a new test file:

- [ ] File name `test_<area>.cpp`.
- [ ] At top: `#include "pae/<area>/Header.hpp"` and `#include <catch2/catch_test_macros.hpp>`.
- [ ] Use `TEST_CASE("<sentence describing behaviour>", "[<labels>]")`.
- [ ] Labels match a CTest filter (`[grid]`, `[algo]`, `[heuristic]`, `[viz]`, `[cross]`, `[property]`, `[perf]`).
- [ ] No global mutable state.
- [ ] No `std::cout` (Catch2 captures `INFO`/`WARN` cleanly).
- [ ] Add a row to the test in `pae/tests/CMakeLists.txt`.
- [ ] Update [`REQUIREMENTS.md`](REQUIREMENTS.md) row if this test
      verifies a previously un-tested requirement.
