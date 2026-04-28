# Algorithms — Pathfinding Analysis Engine

This document is the **algorithmic reference**. It explains how each
pathfinder behaves, what it guarantees, where it costs, and why we
implement it the way we do.

Cross-references:
- Class shapes → [`LLD.md`](LLD.md) §4.
- Heuristics → [`HEURISTICS.md`](HEURISTICS.md).
- Container choices → [`DATA_STRUCTURES.md`](DATA_STRUCTURES.md).

Notation:
- `V` = number of cells (= `width * height`).
- `E` = number of edges out of a cell (= 4 in 4-conn, 8 in 8-conn).
- `b` = effective branching factor (≤ `E`, less in mazes).
- `d` = optimal path depth.

---

## 1. Common scaffolding

All three algorithms share the same skeleton:

```text
gScore[s] = 0
parent[s] = -1
push s onto frontier
while frontier not empty:
    u = pop_best(frontier)
    if u == goal: reconstruct path; return Found
    if visited[u]: continue
    visited[u] = true
    metrics.nodesExpanded++
    viz.onExpand(u)
    for each neighbor v of u:
        if Grid::at(v) == Obstacle: continue
        tentativeG = gScore[u] + edgeCost(u, v)
        if tentativeG < gScore[v]:
            gScore[v]  = tentativeG
            parent[v]  = u
            fScore[v]  = score(tentativeG, v)   ← differs per algo
            push v onto frontier
            metrics.nodesEnqueued++
            viz.onEnqueue(v, fScore[v])
return NotFound
```

The differences between A\*, Dijkstra, and BFS reduce to:

1. **What `pop_best` means** (priority queue vs. FIFO).
2. **What `score(g, v)` is** (`g + h`, `g`, or `depth`).
3. **What `edgeCost` is** (cell weight, cell weight, or always 1).

Everything else — neighbour iteration, reconstruction, metrics
emission, visualizer events — is identical.

In code, we deliberately keep three separate classes (`AStar`,
`Dijkstra`, `BFS`) rather than a single templated `BestFirstSearch<…>`,
for two reasons:

- **Readability and OOP demonstrability.** Each class is short, has a
  single algorithmic intent, and is the canonical answer to "what is
  the smallest meaningful subclass of `IPathfinder`?".
- **Honest naming and metrics.** `Benchmark::Report` rows are labelled
  `dijkstra`, `astar+manhattan`, etc., not `bestfirst<…>`.

We **do** factor out one shared helper: `pae::algo::reconstructPath`,
which walks the `parent` array. That is in
`pae/src/algorithms/internal/Reconstruct.hpp`, used by all three.

---

## 2. Breadth-First Search (BFS)

### 2.1 Intent

Find the path with the **fewest steps** from start to end, ignoring
weights. Equivalent to Dijkstra on a uniformly-weighted graph but
strictly faster because we don't need a heap.

### 2.2 Pseudo-code

```text
queue = FIFO; push start
visited[start] = true
parent[start] = -1
while queue not empty:
    u = queue.pop_front()
    if u == goal: reconstruct; return Found
    for v in neighbors(u):
        if visited[v] or grid.at(v) == Obstacle: continue
        visited[v] = true
        parent[v]  = u
        queue.push_back(v)
return NotFound
```

### 2.3 Properties

| Property | Value |
|----------|-------|
| Optimal | Yes — for **unweighted** edges. |
| Complete | Yes (finite graph). |
| Time complexity | `O(V + V·E) = O(V·E)`. |
| Space complexity | `O(V)` (visited + parent). |
| Container | `std::queue<int32_t>` (deque-backed). |
| Heuristic | None. |

### 2.4 Implementation notes

- We use **cell indices**, not `Coord`, in the queue → the queue holds
  `int32_t`, not 8-byte `Coord`. Halves memory.
- `visited` is a `std::vector<uint8_t>` (true/false per cell). We do
  not use `std::vector<bool>` because element access is slower and
  not by-reference.
- We mark a cell visited **on enqueue**, not on dequeue. This avoids
  duplicate enqueues that would otherwise bloat the queue. (Some
  textbooks mark on dequeue. Marking on enqueue is the correct
  practice for unweighted shortest-path BFS — both produce optimal
  paths but enqueue-marking has tighter memory.)

### 2.5 Edge cases

| Case | Behaviour |
|------|-----------|
| Start == End | `Result{found=true, path={start}, totalCost=0}`. |
| Start surrounded by obstacles | `Result{found=false}`. |
| End surrounded by obstacles | `Result{found=false}`. |
| No path through maze | `Result{found=false}`. |
| Grid 1×1 (only `S` is invalid; ctor would reject — but if forced) | Loader rejects; ctor invariants reject. |

---

## 3. Dijkstra's algorithm

### 3.1 Intent

Find the **minimum-cost** path from start to end on a graph with
**non-negative edge weights**. Cells in our grid have weights ≥ 1;
moving from `u` to `v` costs `weight(v)` (we charge entry into the
destination cell).

### 3.2 Pseudo-code

```text
gScore[*] = +∞;  gScore[start] = 0
heap = min-heap by g; push (start, 0)
parent[start] = -1
while heap not empty:
    (u, gu) = heap.pop()
    if gu > gScore[u]: continue              ← lazy decrease-key
    if u == goal: reconstruct; return Found
    for v in neighbors(u):
        if grid.at(v) == Obstacle: continue
        tentativeG = gScore[u] + grid.weight(v)
        if tentativeG < gScore[v]:
            gScore[v] = tentativeG
            parent[v] = u
            heap.push((v, tentativeG))
return NotFound
```

### 3.3 Properties

| Property | Value |
|----------|-------|
| Optimal | Yes (non-negative weights). |
| Complete | Yes. |
| Time complexity | `O((V + V·E) log V)` with a binary heap. |
| Space complexity | `O(V)` (gScore + parent + heap). |
| Container | `std::priority_queue<…, std::vector<…>, Cmp>`. |
| Heuristic | None. |

### 3.4 Why **lazy decrease-key**?

`std::priority_queue` does not support decrease-key. We could:

1. Use `std::multimap<int64_t, int32_t>` and erase the old entry.
2. Use `boost::heap::fibonacci_heap` (extra dep).
3. **Lazy:** push the new entry; on pop, check `gu > gScore[u]` and
   skip. Stale entries are silently ignored.

Option 3 is the standard choice for grid sizes we care about. The
asymptotic factor is unchanged; constant-factor cost is tiny because
each cell is enqueued at most `E` times (once per relaxation).

### 3.5 Edge cases

Same as BFS, plus:

| Case | Behaviour |
|------|-----------|
| Negative weight | `GridLoader` rejects; `Grid::weight` returns `int32_t ≥ 1`. |
| Disconnected goal | Heap drains; return `Result{found=false}`. |
| Multiple shortest paths | `Result.path` is one of them; selection is deterministic via tie-breaking by smaller cell index on ties. |

---

## 4. A\* search

### 4.1 Intent

Find the minimum-cost path **faster than Dijkstra** by using a
heuristic `h(u)` estimating distance from `u` to goal. Provided `h` is
**admissible** (`h(u) ≤ true_cost(u, goal)`), A\* is optimal.

### 4.2 Pseudo-code

```text
gScore[*] = +∞;  gScore[start] = 0
fScore[start] = h(start, goal)
heap = min-heap by f; push (start, fScore[start])
parent[start] = -1
while heap not empty:
    (u, fu) = heap.pop()
    if fu > fScore[u]: continue              ← lazy decrease-key
    if u == goal: reconstruct; return Found
    for v in neighbors(u):
        if grid.at(v) == Obstacle: continue
        tentativeG = gScore[u] + grid.weight(v)
        if tentativeG < gScore[v]:
            gScore[v] = tentativeG
            fScore[v] = tentativeG + h(v, goal)
            parent[v] = u
            heap.push((v, fScore[v]))
return NotFound
```

### 4.3 Properties

| Property | Value |
|----------|-------|
| Optimal | Yes, iff `h` is admissible. |
| Complete | Yes (finite graph). |
| Time complexity | Worst case `O(b^d)` (no useful heuristic); best case `O(d)` (perfect heuristic). On gridworlds, expansions = θ(L²) where L is the maze "shortcut length"; in practice much fewer than Dijkstra. |
| Space complexity | `O(V)`. |
| Container | `std::priority_queue` keyed on `fCost`. |
| Heuristic | Pluggable `IHeuristic`. |

### 4.4 Admissibility & consistency

- **Admissible:** `h(u) ≤ true_cost(u, goal)` for all `u`. Guarantees
  optimality with the basic A\*.
- **Consistent (monotone):** `h(u) ≤ edge_cost(u, v) + h(v)` for all
  edges `(u, v)`. Guarantees that **once expanded, never re-expanded**
  (so we can use the lazy-decrease-key trick safely without ever
  pulling a stale entry of an already-better cell). All three of our
  heuristics are consistent for their movement model.

If a heuristic is admissible but not consistent, A\* still finds the
optimum but may re-expand nodes; we'd then need a real closed-set
check. Not our case.

### 4.5 Tie-breaking

We break ties on `f` by preferring **larger `g`**:

```cpp
if (a.fCost != b.fCost) return a.fCost > b.fCost;
return a.gCost < b.gCost;   // priority_queue is a max-heap on Cmp's "less"
```

Rationale: among nodes with equal estimated total cost, prefer the one
that has already travelled further — they are "deeper" toward the
goal. This significantly reduces the number of expanded nodes on
open maps without breaking optimality.

A second tie-break (on `f` and `g` equal) is by smaller cell index, to
make the algorithm fully deterministic.

### 4.6 A\* with `h ≡ 0` reduces to Dijkstra

This is the property that lets us write an automatic equivalence test
in `tests/test_cross_algorithm.cpp`:

```cpp
const ZeroHeuristic h0;
AStar     a{ h0 };
Dijkstra  d;
auto ra = a.run(grid, cfg, nullptr, &mA);
auto rd = d.run(grid, cfg, nullptr, &mD);
REQUIRE(ra.totalCost == rd.totalCost);
```

(`ZeroHeuristic` is a test-only `IHeuristic` returning 0.)

### 4.7 Edge cases (specific to A\*)

| Case | Behaviour |
|------|-----------|
| `h` returns `+∞` | Behaves like A\* with that node never popped; harmless. |
| `h` returns negative | We assert `h ≥ 0` in tests; if it slips past, A\* may revisit nodes; algorithm still terminates because the closed check is `gScore` based. |
| `h` overestimates | A\* may return a non-optimal path. We do not use any inadmissible heuristic. |

---

## 5. Differences at a glance

| Feature | BFS | Dijkstra | A\* |
|---------|-----|----------|-----|
| Frontier | FIFO queue | min-heap by `g` | min-heap by `g + h` |
| Edge cost | always 1 | `weight(v) ≥ 1` | `weight(v) ≥ 1` |
| Heuristic | – | – | `IHeuristic` (admissible) |
| Optimal on unweighted | yes | yes | yes |
| Optimal on weighted | **no** | yes | yes (admissible h) |
| Best when | unweighted, small | weighted, no useful h | weighted, useful h |
| Path-cost reported | `path.size() - 1` | `Σ weight(v)` along path | `Σ weight(v)` along path |
| Re-expansion guard | per-cell `visited` | `gu > gScore[u]` | `fu > fScore[u]` |

---

## 6. Complexity summary

| Algorithm | Time | Space | Constant factor |
|-----------|------|-------|-----------------|
| BFS | `O(V·E)` | `O(V)` | Smallest — no heap. |
| Dijkstra | `O((V·E) log V)` | `O(V)` | Heap log overhead per push/pop. |
| A\* | `O((V·E) log V)` worst, `≪` in practice | `O(V)` | Same heap log + per-pop heuristic eval. |

---

## 7. Edge-case test catalogue (must all pass)

Every algorithm is tested against this matrix in
`tests/test_<algo>.cpp`:

| ID | Map | Expected |
|----|-----|---------|
| EC-01 | `tiny.txt` (3×3, S adjacent to E, no obstacles) | `path.size() == 2`, `cost == 1`. |
| EC-02 | `corridor.txt` (1×N straight corridor) | `path.size() == N`, `cost == N-1`. |
| EC-03 | `no_path.txt` (S walled off from E) | `Result{found=false}`. |
| EC-04 | `dense_obstacle.txt` (S=E adjacency blocked, only one detour) | unique known path. |
| EC-05 | `weighted_small.txt` (two paths: short heavy vs long light) | algorithm picks lighter (Dijkstra/A\*) or shorter (BFS). |
| EC-06 | `4x4_open.txt`, S=E | `path.size() == 1`, `cost == 0`. |
| EC-07 | `1024x1024_random.txt` (perf only) | terminates within NFR-2 budget. |

---

## 8. Why **only** these three for v1?

We could add `IDA*`, `JPS`, `Theta*`, etc. We chose A\*/Dijkstra/BFS for
v1 because:

1. **They span the design space.** BFS = unweighted; Dijkstra =
   weighted, no-heuristic; A\* = weighted, heuristic-driven. Adding
   any fourth algorithm becomes a *configuration* (different
   heuristic, different connectivity), not a new design point.
2. **They share the same skeleton.** This makes the OOP demo crisp:
   one interface, three minimally different implementations.
3. **They fit the 6–12 hour budget.** Anything more is V2+ work.

JPS, Theta\*, bidirectional A\* are listed in [`EXTENSIONS.md`](EXTENSIONS.md).
