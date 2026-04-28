# Data Structures — Pathfinding Analysis Engine

The choice of containers is the difference between a correct
pathfinder and a fast correct pathfinder. This document records
**what we use, why, and what we rejected.**

---

## 1. Cheat sheet

| Concern | Container | Why |
|---------|-----------|-----|
| Grid storage | `std::vector<Cell>` (row-major) | Cache-friendly, O(1) index. |
| Per-cell weights | `std::vector<int32_t>` (parallel array, optional) | Same reason; only allocated if grid is weighted. |
| Open set (A\*, Dijkstra) | `std::priority_queue<Node, std::vector<Node>, NodeFCmp>` | Stdlib min-heap; no extra deps. |
| Open set (BFS) | `std::queue<int32_t>` (= `std::deque` under the hood) | FIFO is exactly what BFS needs. |
| Closed set (implicit) | `gScore[idx] != INT64_MAX` | Saves a hash set on the hot path. |
| Per-cell `gScore` | `std::vector<int64_t>` of size V | O(1) per relaxation. |
| Per-cell `parent`  | `std::vector<int32_t>` of size V | Halves the size of any pointer-based parent map. |
| Path output | `std::vector<Coord>` | Owning, contiguous, easy to reverse. |
| Visualizer "open"/"closed" overlays | `std::vector<uint8_t>` | NOT `std::vector<bool>` (see §4.5). |
| Registry | `std::unordered_map<std::string, std::function<unique_ptr<T>()>>` | Lookup-by-name in CLI; <100 entries; perf irrelevant. |

---

## 2. Grid storage — row-major flat vector

```cpp
std::vector<Cell> cells_;     // size = width * height
int32_t toIndex(Coord c) const noexcept { return c.row * width_ + c.col; }
```

**Why row-major?** Neighbour iteration in C++ proceeds left-to-right
within a row, top-to-bottom across rows. Row-major puts horizontally
adjacent cells in adjacent memory addresses, which is exactly what the
prefetcher wants.

**Why a flat vector instead of `vector<vector<Cell>>`?**

| Concern | `vector<vector<Cell>>` | flat `vector<Cell>` |
|---------|------------------------|--------------------|
| Cache locality | Bad — each row is a separate heap allocation. | Single contiguous block. |
| Memory overhead | ~24 bytes per row (vector header) + per-row alloc rounding. | ~24 bytes total. |
| Indexing | `cells[r][c]` — two indirections. | `cells[r*W + c]` — one. |
| Reallocation | Per-row reallocs possible if rows grow. | Single capacity. |

We rejected `vector<vector<>>` outright.

**Why not `std::array<Cell, W*H>` for compile-time-fixed grids?** We
load grids from disk; the size is runtime-known. If we ever build a
fixed-arena variant, that's a different (templated) Grid type — call
it `FixedGrid<W, H>` — outside v1.

---

## 3. Open set — `std::priority_queue` (binary heap)

```cpp
using Heap = std::priority_queue<Node, std::vector<Node>, NodeFCmp>;
```

**Operations needed.** push (relax), pop-min (expand). That's it. We
do **not** need decrease-key; we use the lazy variant
([`ALGORITHMS.md`](ALGORITHMS.md) §3.4).

**Why a binary heap and not a Fibonacci heap or pairing heap?**

| Heap | push | pop | decrease-key | Constants |
|------|------|-----|--------------|----------|
| Binary (stdlib) | O(log V) | O(log V) | O(log V) (lazy) | small, cache-friendly |
| Fibonacci | O(1) amortised | O(log V) | O(1) amortised | large, pointer-chasing |
| Pairing | O(1) | O(log V) | O(log V) amortised | medium |

Asymptotic Fibonacci is "better" for Dijkstra (`O(V log V + E)`
vs. `O((V+E) log V)`), but the constants are ~5–10× higher and the
implementation is non-trivial. On grids up to a few million cells
the binary heap wins by 2–5×. Adding `boost::heap` to the dep tree is
not worth it.

**Why use `vector<Node>` as the underlying container** (the default)?
Because it's contiguous → cache friendly → faster sift-up / sift-down.
We do not switch to `std::deque` or any segmented structure.

**Capacity hint.** We `reserve(V/4)` on the underlying vector before
the search starts. Empirically the open set never grows beyond ~`V/2`
for our maps; a quarter is a generous starting point that avoids the
mid-search reallocation hiccup.

---

## 4. Closed set — **implicit**, via `gScore`

We do **not** allocate a separate `unordered_set<Coord>` or
`vector<uint8_t>` "closed" array. Membership in the closed set is
encoded by:

```cpp
gScore[idx] != std::numeric_limits<int64_t>::max()
```

i.e., "we have settled on a finite cost for this cell." The lazy
heap-pop check (`if (poppedG > gScore[u]) continue;`) handles stale
entries.

**Why this works:** with a *consistent* heuristic, A\* never improves
`gScore[u]` after popping `u`. So checking `gScore[u]` against the
popped node's `g` is equivalent to checking "is `u` already settled?"
— without a second container.

**Saving:** roughly 1 byte per cell, plus the L1/L2 traffic that
container would generate. On a 1024×1024 grid that's 1 MB of L1
pressure removed.

### 4.5 Why **not** `std::vector<bool>` for boolean overlays?

`std::vector<bool>` is the standard library's only "container" that
violates standard container requirements: `operator[]` returns a
proxy, not a `bool&`. This:
- breaks `auto& x = v[i]` (it's a proxy, not a `bool&`).
- prevents many algorithms from working as expected.
- is **slower** at random access on most platforms because of bit-level
  shifting.

We use `std::vector<uint8_t>` and treat 0/1 as boolean. One extra byte
per cell vs. `vector<bool>`'s packed bits is irrelevant at our sizes
and we get sane semantics.

---

## 5. Parent map — `std::vector<int32_t>`

```cpp
std::vector<int32_t> parent_(V, -1);
```

**Why an array, not `std::unordered_map<Coord, Coord>`?**

| Approach | Memory | Lookup |
|----------|--------|--------|
| `unordered_map<Coord, Coord>` | hash bucket + key + value + chain → ~32–48 bytes per entry | ~30 ns per lookup, hash + collision walk |
| flat `vector<int32_t>` | 4 bytes per cell, *all* cells | ~3 ns per lookup, single load |

For grids ≤ 1024×1024 (≈ 1M cells × 4 B = 4 MB) the dense array wins
on both axes. For very sparsely explored gigantic grids (e.g.,
infinite-world planning) the hash map wins; that's a v2 concern.

`int32_t` is enough for any grid we support: 1024×1024 = 2²⁰, well
within `INT32_MAX`. Going to `int64_t` would double memory for no
benefit.

`-1` is the "no parent" sentinel — easy to spot in a debugger; a
plain `0` would be a valid index and ambiguous.

---

## 6. `Node` layout (24 bytes)

```cpp
struct Node {
    int32_t  cellIndex;     // 4 B
    int64_t  gCost;         // 8 B
    double   fCost;         // 8 B
    int32_t  parentCell;    // 4 B
};                          // total 24 B (no padding on most ABIs)
```

We considered storing `gCost` as `int32_t` — saves 4 B but caps total
path cost at ~2 G. With weights up to `10`, we'd hit that ceiling on
grids around 200 M cells. Cheap insurance to use `int64_t`.

We considered making `fCost` `float` — saves 4 B, but Euclidean
heuristics produce non-integer values that need decent precision for
deterministic tie-breaking. `double` ends the discussion.

---

## 7. Path output — `std::vector<Coord>`

Reconstruction walks the `parent_` array backward into a `vector<Coord>`,
then `std::reverse`s it. We could build it forward into a
`std::deque<Coord>` and avoid the reverse, but:
- contiguous storage is friendlier for downstream consumers (CLI
  printing, JSON serialisation).
- the reversal is `O(d)` where `d` is path length — irrelevant compared
  to the search itself.

---

## 8. Sample-map storage — text, not binary

```
20 10
####################
#S................##
#.####.....######.E#
#.#..#.....#......##
...
```

**Why text?** Diff-able in PRs, human-editable, no endianness, no
schema migration. Loader is ≈ 60 LoC and exhaustively tested. For
truly large maps we have `--map-format binary` slated for v0.5 in
[`EXTENSIONS.md`](EXTENSIONS.md), but no map in v1 is large enough to
justify it.

---

## 9. Trade-off table (the decisions, summarised)

| Decision | Picked | Rejected | Why |
|----------|--------|---------|-----|
| Grid storage | flat `vector<Cell>` row-major | nested vectors | cache locality |
| Open set | `priority_queue<Node, vector<Node>>` | Fibonacci heap | constants beat asymptotic |
| Closed set | implicit via `gScore` | `unordered_set<int>` or `vector<bool>` | one fewer container, less memory |
| Boolean overlays (viz) | `vector<uint8_t>` | `vector<bool>` | proxy semantics, perf |
| Parent map | flat `vector<int32_t>` | `unordered_map<Coord, Coord>` | locality + size |
| Coord type | `struct { int32_t row, col; }` | `std::pair<int, int>` | named fields, hashable |
| Cell type | `enum class Cell : uint8_t` | `enum`, `int` | type safety + packing |
| Reconstruction | reverse-walk parent into `vector<Coord>` | front-insert into `deque` | downstream simplicity |
| Map format | ASCII text | binary | diff-able, simple loader |
| Map storage | committed `.txt` files in `pae/maps/` | generated at runtime | reproducibility |

---

## 10. When you're tempted to add a container

Run the checklist:

1. **Can the data live in an existing buffer?** (e.g., closed set lives
   in `gScore`.)
2. **Is the size known at search start?** If yes, prefer
   `vector::reserve` upfront over a dynamic structure.
3. **Is the access pattern random or sequential?** If sequential,
   `vector` or `deque`. Random sparse → `unordered_map`. Random dense
   → `vector` keyed by index.
4. **Does it cross a thread boundary?** v1 is single-threaded; if you
   hit this, stop and discuss in the PR.

If you cannot answer all four, push back on adding the container.
