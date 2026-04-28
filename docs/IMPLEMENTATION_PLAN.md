# Implementation Plan & SDLC — Pathfinding Analysis Engine

> Phased execution plan from a clean repo to a tagged `v1.0.0` release,
> sized so the **vertical slice (V0 → V1)** is implementable by one
> student in **6–12 hours**. Subsequent versions extend without
> changing existing public APIs.

Authoritative architecture: [`ARCHITECTURE.md`](ARCHITECTURE.md).
Authoritative class shapes: [`LLD.md`](LLD.md).
Authoritative requirements: [`REQUIREMENTS.md`](REQUIREMENTS.md).
Long-horizon roadmap: [`ROADMAP.md`](ROADMAP.md).

---

## 1. SDLC model (mirrors SSM / Jyotish)

For every PR (phase task or bugfix):

1. **Plan** — read the relevant section of `LLD.md` + `ALGORITHMS.md`;
   check `FEATURES.md` + `BUGS.md`; pick or open a GitHub issue;
   branch name `feature/F<id>-short-name` or `bugfix/B<id>-short-name`.
2. **Implement** — match file names and class shapes to `LLD.md`
   exactly; respect [agent file-ownership rules](../AGENTS.md#file-ownership-rules).
3. **Test** — `ctest --test-dir pae/build -V`; add Catch2 cases for new
   logic; run sanitizers locally (`cmake -DPAE_SANITIZERS=ON`).
4. **Verify** — clang-tidy + clang-format clean; ASan + UBSan clean;
   benchmarks within tolerance (`/parallel-checks` from
   [`.github/prompts/`](../.github/prompts)).
5. **Document** — update `CHANGELOG.md` (`[Unreleased]`),
   `FEATURES.md` row, `ALGORITHMS.md` if a new algorithm or rule was
   added, `BUGS.md` if a bug was fixed.
6. **Ship** — conventional commit (`feat(algo): add bidir A*`,
   `fix(grid): off-by-one in neighbors4`); PR with issue link;
   reviewer / `@qa` runs `/code-review` + `/parallel-checks`.

Conventional commits — types: `feat`, `fix`, `docs`, `style`,
`refactor`, `test`, `chore`, `perf`, `build`, `ci`. Scopes: `core`,
`io`, `heur`, `algo`, `viz`, `metrics`, `factory`, `cli`, `tests`,
`bench`, `docs`, `ci`, `build`.

---

## 2. Phase map

### Phase V0 — Foundations (≈ 1 hour)

Pure scaffolding. No algorithms yet.

| Step | Deliverable | Owner |
|------|-------------|-------|
| 1 | Top-level CMake delegates to `pae/CMakeLists.txt` | `@build` |
| 2 | `pae` builds an empty executable that prints `--version` | `@build` |
| 3 | Catch2 fetched via FetchContent; `tests/test_smoke.cpp` passes | `@qa` |
| 4 | clang-format + clang-tidy run cleanly on the empty repo | `@build` |
| 5 | CI matrix (Linux, macOS, Windows) green | `@build` |

**Exit gate:** `cmake -S . -B build && cmake --build build && ctest --test-dir build`
succeeds locally and on CI on all three OSes.

---

### Phase V1 — Vertical slice MVP (≈ 5–6 hours, the **6-hour student target**)

Smallest end-to-end product: load a tiny map, run BFS, print the path.

| Step | Deliverable | Owner |
|------|-------------|-------|
| V1-1 | `pae::core::Coord`, `Cell`, `Grid` (basic ctor, `inBounds`, `at`, `toIndex`, `neighbors4`) | `@core` |
| V1-2 | `pae::io::GridLoader::loadFromFile` + `loadFromString` | `@core` |
| V1-3 | `pae::heur::IHeuristic` interface; `Manhattan` impl | `@heuristic` |
| V1-4 | `pae::algo::IPathfinder` interface; `BFS` concrete | `@algorithm` |
| V1-5 | `pae::viz::IVisualizer` + `NullVisualizer` + `CliVisualizer` (`Final` mode only) | `@viz` |
| V1-6 | `pae::metrics::Metrics` (basic counters + wall time) | `@perf` |
| V1-7 | `pae::factory::Registry<T>` + `registerAll()` | `@core` |
| V1-8 | `pae::cli::parseArgs` + `App::run` + `main.cpp` | `@core` |
| V1-9 | `tests/test_grid.cpp`, `test_grid_loader.cpp`, `test_manhattan.cpp`, `test_bfs.cpp` | `@qa` |
| V1-10 | `maps/maze_20x20.txt` + `maps/corridor.txt` + `maps/no_path.txt` | `@core` |

**Exit gate:**

```bash
cd pae
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/pae --map maps/corridor.txt --algo bfs --visualize final
# prints the path; exit code 0
ctest --test-dir build      # all green
```

**Time budget for a student:**

- Project skeleton: 30 min (CMake, headers, namespaces).
- `Grid` + `GridLoader`: 60 min.
- `BFS`: 45 min.
- `Manhattan`: 15 min.
- `CliVisualizer::Final`: 30 min.
- CLI parsing + main: 60 min.
- Tests: 60 min.
- Buffer (debugging, format/tidy fixes, CI): 60 min.

---

### Phase V2 — Algorithm trio + step viz (≈ 3–4 hours)

| Step | Deliverable | Owner |
|------|-------------|-------|
| V2-1 | `Dijkstra` + per-cell weights in `Grid` | `@algorithm`, `@core` |
| V2-2 | `AStar` + `IHeuristic` injection | `@algorithm`, `@heuristic` |
| V2-3 | `Euclidean` heuristic | `@heuristic` |
| V2-4 | `CliVisualizer::Step` with `fpsCap` throttle | `@viz` |
| V2-5 | Cross-algorithm equivalence test (A\* with zero h ≡ Dijkstra) | `@qa` |
| V2-6 | `tests/test_astar.cpp`, `test_dijkstra.cpp`, `test_euclidean.cpp` | `@qa` |
| V2-7 | `pae --benchmark` mode prints comparison table | `@perf` |
| V2-8 | `weighted_small.txt`, `open_arena_50x50.txt`, `maze_50x50.txt` maps | `@core` |

**Exit gate:**
- `pae --map maps/maze_50x50.txt --benchmark` prints a 6-row table
  (3 algos × 2 heuristics for A\*, plus Dijkstra and BFS rows).
- A\* + Manhattan finds the same optimal path cost as Dijkstra on every
  uniform-weight test map.
- All algorithms produce a path within budget on `maps/maze_50x50.txt`.

---

### Phase V3 — Robustness + Chebyshev + 8-conn (≈ 2 hours)

| Step | Deliverable | Owner |
|------|-------------|-------|
| V3-1 | `Chebyshev` heuristic | `@heuristic` |
| V3-2 | 8-conn / diagonal movement support (`--diagonal`) in `Grid::neighbors8` and `IPathfinder` consumers | `@core`, `@algorithm` |
| V3-3 | Tie-breaking deterministic + documented in `LLD.md` §2.3 | `@algorithm` |
| V3-4 | Property tests: heuristic admissibility, symmetry | `@qa` |
| V3-5 | ASan + UBSan jobs in CI | `@qa`, `@build` |
| V3-6 | Edge-case maps: pathological dense obstacle, isolated start, full-blocked goal | `@qa` |

**Exit gate:** all NFR-1 checks green on CI; A\* + Chebyshev on a
diagonally-permitted 8-conn map matches Dijkstra's path cost.

---

### Phase V4 — Performance hardening + reporting (≈ 2 hours)

| Step | Deliverable | Owner |
|------|-------------|-------|
| V4-1 | `Benchmark::sweep` runs N reps with warmup; emits median + p95 | `@perf` |
| V4-2 | `Report::writeCsv` and `writeJson` | `@perf` |
| V4-3 | `bench_pathfinders.cpp` runs against every map in `maps/`; commits CSV summary | `@perf` |
| V4-4 | NFR-2 perf budgets verified in CI on a fixed runner | `@perf`, `@build` |
| V4-5 | `pae --visualize none --benchmark` removes I/O cost from timing | `@viz`, `@perf` |

**Exit gate:** CI benchmark job posts a comment on PRs with the perf
delta vs. `main`. ±2% reproducibility verified.

---

### Phase V5 — Release polish (≈ 1 hour)

| Step | Deliverable | Owner |
|------|-------------|-------|
| V5-1 | `--help`, `--version`, error messages match `REQUIREMENTS.md` §FR-6 | `@core` |
| V5-2 | `docs/CHANGELOG.md` `[1.0.0]` entry | all |
| V5-3 | Tag `v1.0.0`; release workflow attaches Linux/macOS/Windows binaries | `@build` |
| V5-4 | `README.md` quick-start verified by a fresh clone in CI | `@build` |

**Exit gate:** `git tag v1.0.0 && git push --tags` triggers a green
release; downloaded binaries run on the target OSes.

---

## 3. Vertical-slice demos (one command per phase)

```bash
# V0 — foundations
cmake -S . -B build && cmake --build build -j && ctest --test-dir build

# V1 — MVP (BFS, final-only viz)
./build/pae --map pae/maps/corridor.txt --algo bfs --visualize final

# V2 — full trio
./build/pae --map pae/maps/maze_50x50.txt --benchmark

# V3 — diagonal + chebyshev
./build/pae --map pae/maps/open_arena_50x50.txt \
            --algo astar --heuristic chebyshev --diagonal --visualize step

# V4 — reproducible bench
./build/pae --map pae/maps/maze_100x100.txt --benchmark > bench.csv

# V5 — release
git tag v1.0.0 && git push --tags
```

---

## 4. Parallelism map

| Stream A (engine) | Stream B (verification) | Stream C (ops) |
|-------------------|-------------------------|----------------|
| V1 core + BFS | V1 tests for grid & BFS | V0 CMake + CI |
| V2 Dijkstra + A\* + Euclidean | V2 cross-algorithm equivalence tests | V2 benchmark scaffolding |
| V3 Chebyshev + 8-conn | V3 property tests + sanitizers | V3 CI matrix expansion |
| V4 perf-tuning hot loops | V4 perf regression tests | V4 release workflow |

After V0 is done, `@core`/`@algorithm`/`@heuristic` can work in
parallel; `@qa` follows one phase behind `@core`. `@build` is mostly
front-loaded (V0) and back-loaded (V5).

---

## 5. Risk register & escalation

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|-----------|
| Priority queue tie-breaking causes non-determinism in tests | M | M | Deterministic `NodeFCmp` (LLD §2.3); seed-driven secondary tie-break optional. |
| Catch2 v3 download fails in CI | L | M | Cache `_deps/` between runs; pin tag, not branch. |
| Visualization "step" mode floods stdout on big maps | M | L | `fpsCap` + size-based downsample; default to `final` if grid > 200×200. |
| Heuristic implementation accidentally inadmissible | L | H | Property test: random coords, assert `h <= manhattan_actual` for 4-conn maps. |
| Path reconstruction has off-by-one | M | H | `tests/test_astar.cpp` uses fixtures with hand-counted paths. |
| Benchmark numbers vary > 5% | M | M | Use `NullVisualizer` in bench; warmup; median-of-30. |

**Escalation policy** (parity with Jyotish AI §7):

- **Blocked on algorithmic correctness:** open an issue tagged
  `algorithm`, link the conflicting reference; do not merge until a
  reviewer confirms.
- **Blocked on performance regression:** flag with `perf-regression`;
  hold the merge; require either a fix or a justified `BUDGET_WAIVER:`
  comment in `BUGS.md`.
- **Blocked on CI flakiness:** flag with `flaky`; fix the test, do not
  retry-and-merge.

---

## 6. Release cadence (SemVer)

| Release | Content | Tag |
|---------|---------|-----|
| 0.1.0 | V0 + V1 (vertical slice — BFS only) | `v0.1.0` |
| 0.2.0 | V2 (algorithm trio + step viz + 2 heuristics) | `v0.2.0` |
| 0.3.0 | V3 (chebyshev, 8-conn, sanitizers, edge cases) | `v0.3.0` |
| 0.4.0 | V4 (benchmark hardening, CI perf gating) | `v0.4.0` |
| 1.0.0 | V5 (release polish; all NFRs met) | `v1.0.0` |
| 1.x   | extensions from `EXTENSIONS.md` (no API breaks) | minor |
| 2.0.0 | hypothetical breaking change (e.g., 3D grids) | major |

Major = engine output- or CLI-breaking changes. Minor = new algorithm,
new heuristic, new visualizer, new map format. Patch = bug fix.

---

## 7. Definition of Done (per-task)

A task / PR is **Done** when:

- [ ] Code compiles with `-Wall -Wextra -Werror -Wpedantic`.
- [ ] `ctest` is green (Debug + Release locally).
- [ ] clang-tidy reports no new errors.
- [ ] clang-format reports no diff.
- [ ] If algorithmic: a Catch2 test verifies a hand-computed reference.
- [ ] If perf-relevant: the benchmark numbers are recorded in the PR
      description.
- [ ] `docs/CHANGELOG.md` has a new `[Unreleased]` line.
- [ ] `docs/FEATURES.md` row updated to its new state.
- [ ] If a bug: `docs/BUGS.md` row moved to `Fixed` with the commit
      SHA.
- [ ] PR body references the agent that did the work and any deferred
      sub-tasks.
