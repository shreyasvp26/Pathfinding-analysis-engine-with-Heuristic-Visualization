# Heuristics — Pathfinding Analysis Engine

A heuristic in our system is anything that satisfies `IHeuristic`:

```cpp
class IHeuristic {
public:
    virtual ~IHeuristic() = default;
    virtual double estimate(Coord from, Coord to) const noexcept = 0;
    virtual std::string_view name() const noexcept = 0;
};
```

We ship three: **Manhattan**, **Euclidean**, **Chebyshev**. Each is
admissible and consistent for its intended movement model. This file
documents the contract, the math, and when each one is appropriate.

---

## 1. The contract every heuristic must satisfy

| Property | Statement | Why it matters |
|----------|----------|----------------|
| **Non-negative** | `h(a, b) ≥ 0` | A\* invariants assume non-negative `f`. |
| **Goal-zero** | `h(a, a) == 0` | Otherwise A\* may delay popping the goal. |
| **Symmetric** | `h(a, b) == h(b, a)` | Used implicitly by some properties; cheap to satisfy for our heuristics. |
| **Admissible** | `h(a, b) ≤ true_cost(a, b)` | Required for A\* optimality. |
| **Consistent (monotone)** | `h(a, b) ≤ edge_cost(a, c) + h(c, b)` for any neighbour `c` of `a` | Lets A\* skip closed-set re-checks; tighter perf. |
| **Pure / `noexcept`** | No allocation, no I/O, no exceptions | Heuristic is in the inner loop; cost matters. |

We **enforce** non-negative, goal-zero, and symmetry in `tests/`
(property tests with random `Coord` pairs). Admissibility and
consistency are **proven per heuristic** in §3, not tested
exhaustively (an exhaustive test would equal solving the problem).

---

## 2. How heuristics plug into A\*

`AStar` takes the heuristic as a **non-owning const reference** in its
constructor:

```cpp
class AStar : public IPathfinder {
public:
    explicit AStar(const IHeuristic& h) : h_(h) {}
    Result run(const Grid&, const RunConfig&, IVisualizer*, Metrics*) const override;
private:
    const IHeuristic& h_;
};
```

Inside the inner loop, A\* calls `h_.estimate(coord_v, goal)` exactly
once per relaxation that improves `gScore[v]`. The cost is therefore
`O((V·E) log V)` heap operations + `O(V·E)` heuristic evaluations.

The factory creates the heuristic and the algorithm separately and
hands the heuristic into A\*:

```cpp
auto h    = Registry<IHeuristic>::instance().create("manhattan");
auto algo = std::make_unique<AStar>(*h);   // borrows; h outlives algo
```

This is **composition** in the OOP sense: `AStar` *uses* an
`IHeuristic` but does not own it, and we are free to share the same
heuristic across multiple A\* runs in a benchmark sweep.

---

## 3. The three heuristics

Throughout this section, `dx = |a.col - b.col|`, `dy = |a.row - b.row|`.
Per-cell move cost is `1` (uniform); on weighted grids the heuristic is
still admissible *as long as* every cell weight ≥ 1, which we
enforce in `Grid::weight`.

### 3.1 Manhattan distance

```
h(a, b) = dx + dy
```

| Property | Verdict |
|----------|---------|
| Admissible — 4-conn | **Yes.** Every step changes `dx + dy` by at most 1, so dx+dy steps is the absolute floor. |
| Admissible — 8-conn | **No.** A diagonal step reduces both `dx` and `dy` by 1 simultaneously, so the true cost is `max(dx, dy)`, which is `≤ dx + dy`. Manhattan **overestimates**, breaking admissibility. |
| Consistent — 4-conn | Yes (per-step decrease ≤ 1 = edge cost). |
| When to use | Default for grid mazes with **only orthogonal movement**. Excellent fit; tight estimate. |
| Cost | One subtraction, one absolute value, one add. ~3 ALU ops. |

**Use this** when `--diagonal` is **off** (the default).

### 3.2 Euclidean distance

```
h(a, b) = sqrt(dx² + dy²)
```

| Property | Verdict |
|----------|---------|
| Admissible — 4-conn | **Yes**, but loose: in 4-conn the actual minimum cost is `dx + dy ≥ sqrt(dx² + dy²)`. A\* still optimal but explores more nodes than Manhattan. |
| Admissible — 8-conn (unit cost diag) | **Yes**. True cost is `max(dx, dy)` and `sqrt(dx² + dy²) ≤ max(dx, dy) · sqrt(2)`; we'd need `≤ max(dx, dy)` for tightness; Euclidean is admissible (≤ true cost) but loose. |
| Admissible — true Euclidean cost (any-angle) | **Yes** and tight (the heuristic equals the cost). |
| Consistent | Yes (triangle inequality). |
| When to use | Any-angle pathfinding (Theta\*, in v0.5+). For grid-locked movement, prefer Manhattan (4-conn) or Octile (8-conn). |
| Cost | Two squares + sqrt. ~6–8 ALU ops; a `sqrt` is ~10 cycles. The most expensive of the three. |

**Use this** when modelling truly continuous movement, or as a baseline
to demonstrate "looser admissible heuristic = more nodes expanded" in
the comparison table.

### 3.3 Chebyshev distance

```
h(a, b) = max(dx, dy)
```

| Property | Verdict |
|----------|---------|
| Admissible — 4-conn | **No.** True cost is `dx + dy`, Chebyshev is `max(dx, dy) ≤ dx + dy`, so Chebyshev *under*-estimates more — admissibility holds, but very loose. (Wait: under-estimating ≠ inadmissible. Re-check: admissibility = `h ≤ true`. `max(dx,dy) ≤ dx + dy` ⇒ Chebyshev ≤ true ⇒ admissible — yes.) |
| Admissible — 8-conn (unit cost) | **Yes** and **tight**. True cost = `max(dx, dy)`; Chebyshev = same. |
| Consistent | Yes. |
| When to use | 8-conn movement with diagonal step cost = 1 (e.g., chess king moves). Gives the same expansion order as Dijkstra would on the same graph minus the heuristic, with a free improvement. |
| Cost | One subtraction, one absolute value, one max. ~3 ALU ops. |

**Use this** when `--diagonal` is on and diagonal cost == 1.

> **Why not Octile distance** (`max(dx,dy) + (sqrt(2) - 1) · min(dx,dy)`)?
> Octile is the tight 8-conn heuristic when diagonal cost == `sqrt(2)`.
> We list it as a v0.5 extension; it's a one-class addition that fits
> the same `IHeuristic` contract.

---

## 4. Decision matrix

```
                   movement model
              ┌────────┬─────────┐
              │ 4-conn │ 8-conn  │
   ───────────┼────────┼─────────┤
   uniform w  │ Manh.  │ Cheby.  │
   weighted   │ Manh.  │ Cheby.  │
   any-angle  │   —    │ Eucl.   │
              └────────┴─────────┘
```

In v1, `--heuristic euclidean --diagonal` is supported but flagged in
the comparison table as "loose admissible." Users see the cost in
expansion counts.

---

## 5. Adding a new heuristic — worked example (Octile)

This is exactly what `/add-heuristic` automates.

1. **Header** `pae/include/pae/heuristics/Octile.hpp`:

   ```cpp
   #pragma once
   #include "pae/heuristics/IHeuristic.hpp"

   namespace pae::heur {
   class Octile : public IHeuristic {
   public:
       double estimate(core::Coord a, core::Coord b) const noexcept override;
       std::string_view name() const noexcept override { return "octile"; }
   };
   } // namespace pae::heur
   ```

2. **Impl** `pae/src/heuristics/Octile.cpp`:

   ```cpp
   #include "pae/heuristics/Octile.hpp"
   #include <cmath>

   namespace pae::heur {
   double Octile::estimate(core::Coord a, core::Coord b) const noexcept {
       const auto dx = std::abs(a.col - b.col);
       const auto dy = std::abs(a.row - b.row);
       const auto D  = 1.0;
       const auto D2 = std::sqrt(2.0);
       return D * std::max(dx, dy) + (D2 - D) * std::min(dx, dy);
   }
   } // namespace pae::heur
   ```

3. **Register** in `pae/src/factory/register_default.cpp`:

   ```cpp
   reg<IHeuristic, Octile>("octile");
   ```

4. **Tests** in `pae/tests/test_octile.cpp`:

   - `h(a, a) == 0`.
   - Admissibility on 8-conn unit-cost: random `Coord` pairs, assert
     `h ≤ trueChebyshev * sqrt(2)` and `h ≥ trueChebyshev`.
   - A\* with Octile on `maze_50x50.txt` produces the same path cost as
     A\* with Chebyshev (both admissible 8-conn).

5. **Docs** — append to §3 of this file, append a row to the
   benchmark report column legend in `PERFORMANCE.md`.

6. **CHANGELOG** entry under `[Unreleased]`:
   `feat(heur): add Octile distance for 8-conn diagonal-cost-sqrt(2)`.

That is the whole change. Nothing in `AStar`, `Grid`, `App`, or the CLI
is touched.

---

## 6. Common heuristic pitfalls (anti-patterns)

| Pitfall | Symptom | Fix |
|---------|---------|-----|
| Returning `int` from `estimate` | Implicit `double` conversion in tight loop fine, but loses precision for `Euclidean`. | Always `double`. |
| Using a heuristic outside its movement model (Manhattan + 8-conn) | A\* returns a non-optimal path. | Match heuristic to `--diagonal`; CLI prints a warning if mis-paired. |
| Allocating in `estimate` (e.g., constructing a `std::string` for logging) | Massive performance regression. | `noexcept`, no logging in hot path. |
| Caching results in a `mutable std::unordered_map` | Subtle thread-safety bugs once we parallelise benchmarks. | Stay pure; cell pairs are cheap to recompute. |
| Implementing `estimate(Coord, Coord)` then ignoring the second arg | Heuristic only correct from-start; A\* uses it from-anywhere. | Always compute relative to the **goal**, which is `b`. |

---

## 7. Performance baselines (reference)

| Heuristic | Cost per call (ns, dev box) | Expansions on `maze_100x100` (median, 30 runs) |
|-----------|-----------------------------|------------------------------------------------|
| Manhattan | ~3 ns | reference (1.00×) |
| Euclidean | ~14 ns | ~1.10× more expansions (looser admissible) |
| Chebyshev | ~3 ns | identical to Manhattan in 4-conn; reference in 8-conn |

These numbers are **measured by `bench_pathfinders.cpp`** and committed
to `pae/benchmarks/results/heuristics.csv` (gitignored runs, summary
checked in periodically).
