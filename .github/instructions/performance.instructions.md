---
description: 'Auto-applies when editing metrics, benchmarks, or perf-budget tests.'
applyTo: 'pae/**/{metrics,benchmarks}/**'
---

# Performance — coding rules

## Source of truth

- `docs/PERFORMANCE.md` (master).
- `docs/LLD.md` §6 (`Metrics`, `Benchmark`, `Report`).
- `docs/REQUIREMENTS.md` §FR-5, §NFR-2.

## Timing

```cpp
using clock = std::chrono::steady_clock;
const auto t0 = clock::now();
// … work …
const auto t1 = clock::now();
metrics.wallMicros =
    std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
```

- **`steady_clock`**, never `system_clock`.
- **Microseconds.**
- One read at start, one at end. No per-iteration timing.

## Benchmark

- Warmup ≥ 3 reps before measurement.
- Sample size ≥ 30 reps.
- Discard min and max only if explicitly trimmed-mean variant; default
  reports raw median, p95, min, max, stddev.
- `NullVisualizer` always.
- `Metrics` struct **reused** across reps with `reset()`, not
  reallocated.

## Aggregation

```cpp
struct Aggregate {
    int64_t median;
    int64_t p95;
    int64_t min;
    int64_t max;
    double  stddev;
};
```

p95 = nearest-rank on a sorted vector. Median = midpoint (or average
of two middles for even N).

## Reproducibility budget

- `nodesExpanded`, `nodesEnqueued`, `pathLength`, `pathCost`,
  `approxPeakBytes`: **byte-exact** between runs (no jitter).
- `wallMicros`: **median within ±2%** of the committed baseline on
  the same runner type.

A regression > 5% on `wallMicros.median` blocks the PR unless the
description contains `BUDGET_WAIVER:` and a baseline-update PR is
linked.

## Reporting formats

- **Console table** (`Report::printTable`): right-aligned numbers,
  thousands separator (apostrophes — locale-stable). Stable column
  order.
- **CSV**: header on first line, `\n` row separator, no UTF BOM.
- **JSON**: schema in `docs/PERFORMANCE.md` §4. No trailing commas.

## Anti-patterns

| Anti-pattern | Why it's wrong |
|--------------|---------------|
| Reading `clock::now()` per iteration | Timer dominates real cost on small grids. |
| Forgetting `NullVisualizer` in `sweep` | I/O drowns the timings. |
| Comparing baselines across runner types | Phantom regressions. Baselines are namespaced by runner. |
| Reporting only `mean` | Median + p95 is what we trust. Mean is misleading on skewed distributions. |
| Mutating shared state across reps | Implies a memoisation that biases later reps. Always `reset()`. |
| Benchmarking in Debug build | Numbers are 5–20× slower; meaningless. CI's perf job is Release-only. |
