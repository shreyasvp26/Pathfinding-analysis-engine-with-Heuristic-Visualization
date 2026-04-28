# Performance Analysis Plan — Pathfinding Analysis Engine

The whole point of this engine is **measurable comparison**. This
document describes how we measure, what we measure, and what the
acceptance budgets are.

Cross-references:
- Container choices that drive constants → [`DATA_STRUCTURES.md`](DATA_STRUCTURES.md).
- Algorithmic complexity → [`ALGORITHMS.md`](ALGORITHMS.md) §6.
- Heuristic per-call cost → [`HEURISTICS.md`](HEURISTICS.md) §7.
- NFRs → [`REQUIREMENTS.md`](REQUIREMENTS.md) §NFR-2.

---

## 1. What we measure (the `Metrics` schema)

Every algorithm run populates a single `Metrics` struct:

```cpp
struct Metrics {
    int64_t nodesExpanded   = 0;   // pop-from-frontier count
    int64_t nodesEnqueued   = 0;   // push-into-frontier count
    int64_t pathLength      = 0;   // number of cells in returned path
    int64_t pathCost        = 0;   // sum of weights along path
    int64_t wallMicros      = 0;   // std::chrono::steady_clock
    int64_t approxPeakBytes = 0;   // analytical estimate; not RSS
};
```

| Metric | Algorithm-specific meaning |
|--------|----------------------------|
| `nodesExpanded` | Counts only nodes popped *and not skipped* by the lazy-stale check. The honest "we did real work on this node" count. |
| `nodesEnqueued` | Includes stale entries pushed before a relaxation; matches the real heap operation count. |
| `pathLength` | `Result::path.size()`. |
| `pathCost` | Sum of `Grid::weight(c)` along the path (BFS reports this as `path.size() - 1` to make BFS row comparable). |
| `wallMicros` | Time between just-before `run()` returns its `Result` and just-after the search loop terminates. **Excludes** Grid loading and Result construction. |
| `approxPeakBytes` | `sizeof(Node) * peak_open_size + sizeof(int64_t) * V + sizeof(int32_t) * V`. Computed at end of run; not OS-level. |

We do **not** record CPU time, page faults, instruction counts, or
cache misses in v1. Those are out of scope (and require platform-
specific instrumentation; see [`EXTENSIONS.md`](EXTENSIONS.md)).

---

## 2. How we measure runtime

```cpp
using clock = std::chrono::steady_clock;
const auto t0 = clock::now();
// … the algorithm body …
const auto t1 = clock::now();
metrics.wallMicros =
    std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
```

Notes:
- **`steady_clock`**, not `system_clock`. We do not want NTP jitter in
  our metrics.
- **Microseconds** because algorithm runs on small maps are sub-
  millisecond. Nanoseconds are noise on most platforms.
- **Visualizer is a `NullVisualizer`** during benchmark mode. CLI I/O
  would dominate timings on large grids.
- The clock is read **once at start, once at end** — no timestamps
  per node. Per-node timing would be both noisy and meaningless.

---

## 3. How we compare algorithms (`Benchmark::sweep`)

For a given map, the benchmark performs:

```
for each (algorithm, heuristic) in product:
    if algorithm needs no heuristic and heuristic != "":
        skip   (e.g., dijkstra+manhattan is not a thing)

    # Warmup: avoid JIT/cache-cold biases
    for r in 1..3:
        algo.run(grid, cfg, NullVisualizer, dummyMetrics)

    # Real samples
    for r in 1..N (default 30):
        m = Metrics{}
        algo.run(grid, cfg, NullVisualizer, &m)
        samples.push_back(m)

    report.row(algorithm, heuristic).aggregate(samples)
        .median, .p95, .min, .max, .stddev
```

The `Report` is a table:

```
map: maze_100x100.txt   (size: 10000 cells, walkable: 8312)

algorithm   heuristic   expanded     wall_us   path_len  path_cost   memory
─────────   ─────────   ──────────   ───────   ────────  ─────────   ──────
astar       manhattan        2 184    13 412        117        116    312KB
astar       euclidean        2 391    14 891        117        116    312KB
astar       chebyshev        2 184    13 503        117        116    312KB
dijkstra        —            8 015    47 122        117        116    312KB
bfs             —            8 015    18 884        117        117      —
```

(`memory` reported only for heap-using algorithms; BFS row shows `—`
for working-set memory because the queue is naturally O(V).)

---

## 4. How we **log** results

Three sinks are supported; the `Report` class can emit any of them:

| Sink | Method | Used by |
|------|--------|---------|
| Pretty console table | `Report::printTable(ostream&)` | default for `pae --benchmark` |
| CSV | `Report::writeCsv(ostream&)`   | `pae --benchmark --format csv` |
| JSON | `Report::writeJson(ostream&)` | `pae --benchmark --format json`; consumed by CI commenter |

CSV columns: `map,algorithm,heuristic,expanded,enqueued,wall_us,path_len,path_cost,approx_peak_bytes,reps,median,p95,min,max,stddev`.

JSON shape:

```json
{
  "map": "pae/maps/maze_100x100.txt",
  "config": { "diagonal": false, "reps": 30, "warmup": 3 },
  "runs": [
    {
      "algorithm": "astar",
      "heuristic": "manhattan",
      "samples": 30,
      "metrics": {
        "expanded":  { "median": 2184, "p95": 2210, ... },
        "wall_us":   { "median": 13412, "p95": 14820, ... },
        "path_len":  117,
        "path_cost": 116
      }
    },
    ...
  ]
}
```

---

## 5. Reproducibility budget (NFR-2)

- The same algorithm + same heuristic + same map + same `--seed` must
  produce the same `Result.path`, `pathCost`, `nodesExpanded`,
  `nodesEnqueued`, byte-for-byte.
- `wallMicros` will vary; **median over 30 samples must be within ±2%
  of the canonical baseline** stored in `pae/benchmarks/baselines/`.

CI enforces this: a job runs the benchmark, parses the JSON, compares
the `expanded` and `path_cost` to the baseline (must match exactly),
and the `wallMicros.median` to the baseline (must be within ±2% on
the runner type pinned in `ci.yml`).

A `>5%` regression is a **PR-blocking** failure unless the PR description
contains an explicit `BUDGET_WAIVER: <reason>` line and an updated
baseline in the same commit.

---

## 6. Performance budgets (NFR-2.1 → NFR-2.4 restated)

| Map | Algo | Heuristic | Median wall (Release, x86_64 4 GHz dev box) |
|-----|------|-----------|---------------------------------------------|
| `maps/maze_100x100.txt`     | A\*       | Manhattan | < 50 ms |
| `maps/maze_100x100.txt`     | BFS       | —         | < 100 ms |
| `maps/weighted_100x100.txt` | Dijkstra  | —         | < 200 ms |

Memory budget:

| Grid | Working set (`gScore + parent + open peak`) |
|------|---------------------------------------------|
| 1024 × 1024 | < 20 MB |

(Grid storage itself is ~1 MB, separate.)

---

## 7. The constants we deliberately optimise

[`DATA_STRUCTURES.md`](DATA_STRUCTURES.md) §9 lists *what* we picked.
Here we list *how much it matters* in measured ns/op on a reference
box (Apple M1, clang 17 -O3):

| Operation | Picked | Alternative | Speedup |
|-----------|--------|------------|---------|
| Cell read | flat `vector<Cell>` | nested `vector<vector<>>` | ~1.6× |
| Heap push | `priority_queue<Node>` | `multimap<int64_t,int>` | ~3× |
| Closed check | `gScore != INT64_MAX` | `unordered_set<int>` | ~5× |
| Heuristic call | `noexcept`, scalar | virtual + log + cache | irrelevant if pure; >10× if logging/locking |
| Path reverse | `std::reverse` on vector | `deque::push_front` loop | ~2× |

Cumulatively: A\* + Manhattan on `maze_100x100.txt` runs ~3× faster than
a naïve student implementation that uses `unordered_set<Coord>` for
closed and `unordered_map<Coord,Coord>` for parents. Documented in
`pae/benchmarks/results/baseline_naive.csv`.

---

## 8. Methodology

### 8.1 Hardware classes

The CI baseline is `ubuntu-latest` × `c5.xlarge`-class GitHub-hosted
runners. Local development boxes will be faster (Apple M-series) or
slower (older laptops); the **CI baseline is the contract**.

### 8.2 Compiler flags

Release config:

```
-O3 -DNDEBUG -fno-plt
```

We do **not** use `-march=native` for the published baseline (would
make the CI binary non-portable). Local dev may add it via
`-DPAE_NATIVE=ON`.

### 8.3 Anti-noise

| Source of noise | Mitigation |
|-----------------|-----------|
| First-call cache misses | 3 warmup runs, discarded. |
| OS scheduler jitter | Median-of-30 + p95 reported. |
| Power state ramp-up | Runner is fixed-frequency on CI; informational on local. |
| Visualizer I/O | `NullVisualizer` during benchmark. |
| Heap fragmentation | Each `run()` is in its own scope; `Grid` is loaded once and shared. |

### 8.4 What a "regression" looks like

| Symptom | Probable cause | Investigation |
|---------|----------------|---------------|
| `nodesExpanded` differs from baseline | Algorithm semantics changed (e.g., tie-breaking) | Compare commits; was a heuristic, neighbour iteration, or `NodeFCmp` modified? |
| `wallMicros` up >5%, `nodesExpanded` unchanged | Constant-factor regression (data structure, allocation, branch predict) | Profile (perf, Instruments) the inner loop. |
| `wallMicros` up, `nodesEnqueued` up | Looser heuristic, less effective tie-break, or wrong movement model. | Check heuristic + diagonal flag. |
| Memory regression | New container introduced, missing `reserve()`, or leak. | Re-run with ASan; check for `make_unique` in hot loop. |

---

## 9. Where the data lives

```
pae/benchmarks/
├── CMakeLists.txt
├── bench_pathfinders.cpp          — sweep over algos × heurs × maps
├── baselines/                     — committed reference numbers
│   ├── ubuntu-c5xl/
│   │   ├── maze_100x100.json
│   │   └── weighted_100x100.json
│   └── README.md                  — runner-class disclosures
└── results/                       — local runs (gitignored)
```

Baselines are updated in a deliberate PR titled
`bench: refresh baseline (<runner>, <reason>)`. The diff is reviewed
manually; we do not auto-update baselines.

---

## 10. Engineering rule of thumb

**If a change affects the perf profile, the PR description must include
a benchmark diff.** This is enforced socially (review checklist in
`PULL_REQUEST_TEMPLATE.md`) and mechanically (the perf-budget CI job).

A useful local incantation:

```bash
cd pae
cmake -S . -B build-rel -DCMAKE_BUILD_TYPE=Release
cmake --build build-rel -j
./build-rel/benchmarks/pae_bench --json > before.json
git checkout my-branch
cmake --build build-rel -j
./build-rel/benchmarks/pae_bench --json > after.json
python scripts/perf_diff.py before.json after.json
```

(`perf_diff.py` is a tiny script that prints percentage differences
per metric; lives in `pae/scripts/`.)
