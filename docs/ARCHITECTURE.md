# Architecture — Pathfinding Analysis Engine

This document is the **system overview**. The matching detailed design
(class shapes, methods, signatures, invariants) lives in
[`LLD.md`](LLD.md). The phasing of how we get there lives in
[`IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md).

---

## 1. Goals (one paragraph)

We build a single C++17 binary that takes a 2D grid map and a chosen
algorithm + heuristic, runs the search, optionally visualises the
search step-by-step, and emits a metrics record (nodes expanded, path
length, wall time, approximate memory). The same binary can run a
**benchmark mode** that runs every algorithm on the same map and prints
a comparison table. The architecture is OOP-first: every replaceable
piece — algorithm, heuristic, visualizer, map loader — is behind an
interface, registered in a central factory, and selected at runtime
from a CLI string.

---

## 2. High-level architecture

```
                   ┌─────────────────────────────┐
                   │           CLI               │
                   │  parses argv → AppConfig    │
                   └──────────────┬──────────────┘
                                  │
                                  ▼
                   ┌─────────────────────────────┐
                   │       Application           │
                   │  loads grid, picks algo,    │
                   │  picks heuristic, picks viz │
                   └──┬──────────────┬───────────┘
                      │              │
        ┌─────────────┘              └────────────────┐
        ▼                                             ▼
┌───────────────┐  IPathfinder  ┌──────────────────────────────┐
│   Grid (RO)   │ ────────────► │   Algorithm (AStar/Dijkstra/ │
│   Coord       │               │              BFS)            │
│   Cell        │               │   uses → IHeuristic          │
└───────────────┘               │   emits → IVisualizer events │
                                │   writes → Metrics           │
                                └────────┬─────────────────────┘
                                         │
                                         ▼
                  ┌────────────────────────────────────────┐
                  │   IVisualizer (CliVisualizer / Null)   │
                  │   prints step-by-step or final path    │
                  └────────────────────────────────────────┘
                                         │
                                         ▼
                  ┌────────────────────────────────────────┐
                  │   Metrics + Benchmark report           │
                  └────────────────────────────────────────┘
```

Each rounded box is a **library target** in CMake; arrows are
`target_link_libraries` edges. The CLI binary depends on
`pae_app` which depends on every library. Libraries depend
**only inward** (no library depends on the CLI).

---

## 3. Components and responsibilities

### 3.1 `pae_core` (library)

Domain primitives only — no algorithms, no I/O.

| Type | Responsibility |
|------|---------------|
| `Coord` | Integer 2D coordinate. Hashable. Trivially copyable. |
| `Cell` | Enum: `Empty`, `Obstacle`, `Start`, `End`. |
| `Node` | Search-frontier record (coord, gCost, fCost, parentIndex). Used by algorithms; not by the grid itself. |
| `Grid` | 2D array of `Cell`s + per-cell weights + start + end. **Immutable after construction.** Provides bounds-checked access, neighbour iteration (4-conn or 8-conn), weight lookup. |

**Invariant:** A `Grid` always has exactly one Start and one End cell;
constructor enforces this.

### 3.2 `pae_io` (library)

Serialization only.

| Type | Responsibility |
|------|---------------|
| `GridLoader` | Reads ASCII grid files (`.`, `#`, `S`, `E`, optional `0-9` for weight). Validates dimensions, character set, single Start/End. Returns `Grid` by value (move). |
| `GridDumper` | (optional) Writes a `Grid` back to disk for fixture generation. |

### 3.3 `pae_heuristics` (library)

| Type | Responsibility |
|------|---------------|
| `IHeuristic` | Abstract base. `double estimate(Coord, Coord) const noexcept = 0;`. Must be admissible (≤ true cost). |
| `Manhattan` | `\|dx\| + \|dy\|`. Admissible iff diagonal moves are disallowed. |
| `Euclidean` | `sqrt(dx² + dy²)`. Always admissible. |
| `Chebyshev` | `max(\|dx\|, \|dy\|)`. Admissible for 8-conn unit-cost. |

### 3.4 `pae_algorithms` (library)

| Type | Responsibility |
|------|---------------|
| `IPathfinder` | Abstract base. `Result run(const Grid&, const RunConfig&, IVisualizer*, Metrics*) const = 0;`. |
| `AStar` | Best-first search using `f = g + h`. Holds a non-owning pointer to an `IHeuristic`. |
| `Dijkstra` | Uniform-cost search. No heuristic. |
| `BFS` | Step-counting search. Ignores weights. |
| `RunConfig` | Runtime tuning: `diagonal`, `tieBreaking`, `seed`. |
| `Result` | `{ found: bool, path: vector<Coord>, totalCost: int64_t }`. |

### 3.5 `pae_visualization` (library)

| Type | Responsibility |
|------|---------------|
| `IVisualizer` | Abstract base. Receives `onEnqueue`, `onExpand`, `onPathFound`, `onSearchComplete` events. |
| `CliVisualizer` | ANSI-colour renderer. Modes: `none`, `final`, `step`. |
| `NullVisualizer` | No-op (used in benchmarks to remove I/O cost from timing). |

**Key design:** algorithms call **through** the interface; they have no
knowledge of terminals, colours, or sleep delays. The visualizer
chooses how (and how often) to render.

### 3.6 `pae_metrics` (library)

| Type | Responsibility |
|------|---------------|
| `Metrics` | POD struct: `nodesExpanded`, `nodesEnqueued`, `pathLength`, `pathCost`, `wallMicros`, `approxPeakBytes`. |
| `Benchmark` | Runs N×M sweeps (algorithms × heuristics) on a fixed map, returns a `Report`. Stat-aware (median, p95). |
| `Report` | Pretty-prints a comparison table; can also emit CSV/JSON. |

### 3.7 `pae_factory` (library)

| Type | Responsibility |
|------|---------------|
| `Registry<TInterface>` | Templated string→factory map. Used by `Registry<IPathfinder>` and `Registry<IHeuristic>`. |
| `registerAll()` | One-call function (built-in registrations live here, not in algorithm/heuristic translation units, to keep them dependency-free). |

This is the **only** place that knows the names `"astar"`, `"dijkstra"`,
`"bfs"`, `"manhattan"`, etc. CLI uses `Registry<…>::create(name)`.

### 3.8 `pae_app` + `pae` (binary)

| Type | Responsibility |
|------|---------------|
| `cli::parseArgs` | Argv → `AppConfig`. Pure function (testable). |
| `App::run` | Orchestrates one execution: load grid, build pathfinder, build heuristic, build visualizer, call `IPathfinder::run`, print metrics, return exit code. |
| `main.cpp` | 5 lines: parse → run → return. |

---

## 4. Data flow (single run)

```
argv
  │
  ▼ parseArgs
AppConfig { mapPath, algo, heuristic, visualize, … }
  │
  ▼ GridLoader::load
Grid (RO)
  │
  ▼ Registry<IHeuristic>::create
unique_ptr<IHeuristic>
  │
  ▼ Registry<IPathfinder>::create
unique_ptr<IPathfinder>      ┐
                              ├──► IPathfinder::run(grid, cfg, viz*, metrics*)
unique_ptr<IVisualizer>       │       (events flow into viz; values flow into metrics)
                              │
unique_ptr<Metrics>           ┘
  │
  ▼ App
print metrics, return exit code
```

For `--benchmark`, the same flow is run inside `Benchmark::sweep` over
the cross product of registered algorithms and heuristics; results
land in a `Report` instead of being printed inline.

---

## 5. Module dependency graph

```
                  ┌─────────────┐
                  │  pae (bin)  │
                  └──────┬──────┘
                         │
                  ┌──────▼──────┐
                  │  pae_app    │
                  └──────┬──────┘
              ┌──────────┼─────────────┬───────────┬───────────┐
              ▼          ▼             ▼           ▼           ▼
        ┌─────────┐ ┌──────────┐ ┌──────────┐┌───────────┐┌────────┐
        │ algo's  │ │heuristics│ │   viz    ││  metrics  ││factory │
        └────┬────┘ └─────┬────┘ └────┬─────┘└────┬──────┘└───┬────┘
             │            │           │           │            │
             ▼            ▼           ▼           ▼            ▼
                            ┌────────────┐
                            │  pae_core  │
                            └─────┬──────┘
                                  ▼
                            ┌────────────┐
                            │   pae_io   │
                            └────────────┘
```

Hard rule: **`pae_core` and `pae_io` depend on nothing else.** Heuristics
depend only on `pae_core`. Algorithms depend on `pae_core`, `pae_metrics`,
and the `IHeuristic` and `IVisualizer` interfaces (header-only). Visualizers
depend on `pae_core`. Metrics depend on `pae_core`. The factory depends on
the abstract interfaces but **not** on concrete classes — concrete
registrations live in a separate `pae_factory_default` translation unit
that depends on everything.

---

## 6. Process / threading model

V1 is **single-threaded**. All algorithms run on the main thread.
There is no async I/O, no thread pool, no shared mutable state.

Future (v0.4 stretch): the `Benchmark::sweep` may run independent
algorithm runs in parallel (`std::async`). When/if introduced, the
constraint is that **algorithms remain single-threaded internally**;
parallelism is at the run level, not the search level.

---

## 7. Memory model

- Grids are stored row-major in a single `std::vector<Cell>` of size
  `width × height` for cache locality.
- Algorithms allocate two buffers per run: a `gScore` array
  (`vector<int64_t>` of size `width × height`, default `INT64_MAX`)
  and a `parent` array (`vector<int32_t>`, default `-1`).
- The open set is a `std::priority_queue<Node, vector<Node>, NodeCmp>`.
  Closed-set membership is implicit via `gScore[idx] != INT64_MAX`.
  No separate hash set is used.
- Path reconstruction walks the `parent` array backward and reverses
  the result.

This makes per-cell memory: `1 (Cell) + 1 (weight, optional) + 8 (gScore) + 4 (parent) ≈ 14 bytes`.
1024×1024 grid ≈ 14.7 MB working set — well under the NFR-2.4 budget after
removing the per-cell weight (which stays in `Grid`, shared, not duplicated
per algorithm run).

---

## 8. Error handling strategy

| Class | Throws | Returns |
|-------|--------|---------|
| `GridLoader` | `pae::IoError` (file missing, malformed) | `Grid` (move) |
| `Grid::at` | `std::out_of_range` (bounds violation) | `Cell` |
| `Registry::create` | `pae::UnknownNameError` | `unique_ptr<T>` |
| `IPathfinder::run` | does **not** throw on "no path" — returns `Result{found=false}` | `Result` |
| `App::run` | catches all `pae::Error`, maps to non-zero exit code | `int` |

Algorithms throwing on no-path would conflate "expected absence" with
"engine bug." We keep the difference explicit at the boundary.

---

## 9. Extensibility playbook (worked example)

To add **Bidirectional A\***:

1. `@algorithm`: create
   `pae/include/pae/algorithms/BidirectionalAStar.hpp` and
   `.cpp`. Inherit `IPathfinder`. Implement `run`.
2. `@core`: register it in `pae/src/factory/register_default.cpp`:
   `Registry<IPathfinder>::reg("bdastar", []{ return std::make_unique<BidirectionalAStar>(); });`.
3. `@qa`: add `tests/test_bdastar.cpp` — equivalence test against
   Dijkstra, optimality on reference maps.
4. `@perf`: add it to the benchmark sweep set in `bench_pathfinders.cpp`.
5. Update `docs/ALGORITHMS.md` and `docs/CHANGELOG.md`.

**Nothing else changes.** No CLI code, no Grid code, no heuristic code.
That is the point.

---

## 10. Where to read next

- [`LLD.md`](LLD.md) — every class with full method signatures, ownership
  rules, and complexity per call.
- [`ALGORITHMS.md`](ALGORITHMS.md) — algorithmic deep dive, pseudo-code,
  proofs.
- [`DATA_STRUCTURES.md`](DATA_STRUCTURES.md) — why `priority_queue` and
  not a Fibonacci heap, why row-major, etc.
- [`IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md) — task ordering,
  milestones, vertical slices.
