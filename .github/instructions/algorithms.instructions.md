---
description: 'Auto-applies when editing pathfinding algorithms (A*, Dijkstra, BFS).'
applyTo: 'pae/**/algorithms/**'
---

# Algorithms — coding rules

## Source of truth

- Class shapes: `docs/LLD.md` §4.
- Behaviour: `docs/ALGORITHMS.md`.
- Tie-breaking: `docs/LLD.md` §2.3.
- Container choices: `docs/DATA_STRUCTURES.md`.

If the code disagrees with these documents, the documents win.

## The skeleton

Every algorithm follows the skeleton in `docs/ALGORITHMS.md` §1. Do
not invent a new control flow without an LLD update first.

```cpp
Result run(const Grid& grid, const RunConfig& cfg,
           IVisualizer* viz, Metrics* metrics) const override {
    // 1. Setup buffers (no allocations later).
    const int32_t V = grid.width() * grid.height();
    std::vector<int64_t> gScore(V, std::numeric_limits<int64_t>::max());
    std::vector<int32_t> parent(V, -1);

    // 2. Seed start.
    const int32_t startIdx = grid.toIndex(grid.start());
    gScore[startIdx] = 0;

    // 3. Frontier (priority_queue / queue).
    Heap heap;
    heap.push(/* algorithm-specific seed Node */);

    // 4. Search loop.
    while (!heap.empty()) {
        // … pop, lazy-stale check, expand, relax neighbours …
        if (viz) viz->onExpand(grid.toCoord(uIdx));
        if (metrics) ++metrics->nodesExpanded;
    }

    // 5. Reconstruction (or NotFound).
}
```

Reconstruction: use the helper in
`pae/src/algorithms/internal/Reconstruct.hpp`.

## Hot-loop rules (zero compromise)

1. **No allocations.** `gScore` and `parent` are sized once at start.
   Heap container is `std::vector<Node>` with a `reserve(V/4)`.
2. **No `unordered_set` for closed.** Implicit via
   `gScore != INT64_MAX` (consistent heuristic) or via the lazy-stale
   `if (poppedG > gScore[u]) continue;` check.
3. **No `std::function` or other type-erased callbacks** inside the
   loop. The heuristic is `const IHeuristic&` (one virtual call per
   relaxation; acceptable).
4. **`viz` and `metrics` may be `nullptr`.** Check once per
   call site, not per loop iteration if you can hoist.
5. **Single steady_clock read at start, single at end.** Don't measure
   per-iteration timings.

## Tie-breaking — full spec

```cpp
struct NodeFCmp {
    bool operator()(const Node& a, const Node& b) const noexcept {
        if (a.fCost != b.fCost) return a.fCost > b.fCost;
        if (a.gCost != b.gCost) return a.gCost < b.gCost;   // prefer larger g
        return a.cellIndex > b.cellIndex;                   // final determinism
    }
};
```

This makes the priority queue a min-heap on `f`, secondarily a
max-heap on `g`, finally tie-broken by smaller cell index.

## Diagonal / 8-conn

- Only when `cfg.diagonal == true`.
- Diagonal cost = `1` (Chebyshev model). `sqrt(2)` mode is a
  v1.x extension and **not** in v1.
- A\* + Manhattan + diagonal is **rejected at the CLI** (Manhattan is
  inadmissible for 8-conn). The algorithm itself does not validate;
  the CLI does.

## What goes in `Result`

```cpp
struct Result {
    bool                 found{false};
    std::vector<Coord>   path;
    int64_t              totalCost{0};
};
```

- `found = true` ⇒ `path` non-empty, `path.front() == start`,
  `path.back() == end`.
- `found = false` ⇒ `path` empty, `totalCost == 0`.

## What you write to `Metrics`

```cpp
metrics->nodesExpanded   = …;
metrics->nodesEnqueued   = …;
metrics->pathLength      = result.path.size();
metrics->pathCost        = result.totalCost;
metrics->wallMicros      = …;
metrics->approxPeakBytes =
    static_cast<int64_t>(sizeof(Node)) * peakOpenSize +
    static_cast<int64_t>(sizeof(int64_t)) * V +          // gScore
    static_cast<int64_t>(sizeof(int32_t)) * V;           // parent
```

`peakOpenSize` is tracked alongside the heap: `if (heap.size() > peak) peak = heap.size();`.

## Visualizer events

- `onSearchStart(grid)` — once at the top of `run`.
- `onEnqueue(coord, fCost)` — every push.
- `onExpand(coord)` — every accepted pop (after lazy-stale skip).
- `onPathFound(path)` — exactly once, on success.
- `onSearchComplete(metrics)` — once at the bottom (after metrics
  finalised).

Throttling is the visualizer's job, not yours.

## Anti-patterns

| Anti-pattern | Why it's wrong |
|--------------|---------------|
| `std::set<std::pair<double, Coord>>` for the open set | Allocates per push; tree-walks on pop; pointer-chasey. |
| `std::unordered_map<Coord, double>` for `gScore` | 5× slower than flat `vector<int64_t>` keyed by index. |
| `auto path = std::move(reconstructed); path.push_front(start);` | `vector::push_front` doesn't exist; `std::deque` does, but reverse-and-pop is cheaper. |
| `if (viz) viz->onExpand(c); else { /* nothing */ }` per iteration with `viz` known nullptr | Hoist to a `if (viz)` check, or always pass `NullVisualizer`. The factory does the latter. |
| Storing `parent` as `Coord` instead of `int32_t` | Doubles memory; index is canonical anyway. |
