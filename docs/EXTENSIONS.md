# Extensions — Pathfinding Analysis Engine

Optional but valuable additions. None of these is a v1 commitment;
each is shaped to **fit the existing OOP contract** so adding it is a
single agent's worth of work, not an architectural upheaval.

Each extension lists:
- the new public surface,
- the existing classes it modifies (which should be **none** ideally),
- the rough effort budget,
- the agent that would own it,
- the risk that pushes it out of v1.

---

## 1. Weighted grids — already partly in v1

**Status:** v1 supports per-cell weights. v0.2 of the *map format*
extends the loader to accept `0`–`9` digits in cells (with `1+digit`
treated as the weight) and to parse a `# weights inline` header
sentinel.

**Surfaces:** `GridLoader::loadFromFile` (parser), `Grid::weight` (already
exists). No algorithm changes (Dijkstra/A\* already use `weight`; BFS
ignores it by contract).

**Effort:** ~1 hour.
**Owner:** `@core`.
**Risk:** zero — additive, backward compatible.

---

## 2. Diagonal movement (8-conn) — already partly in v1

**Status:** v1 ships `Grid::neighbors8`; the v3 phase wires it through
`IPathfinder` consumers and the CLI flag `--diagonal`.

**Diagonal cost.** Two choices:
- `1` (Chebyshev model — chess king),
- `sqrt(2)` (Euclidean model — natural geometry).

Default is `1`. `--diag-cost sqrt2` switches; this requires algorithms
to use `double` `gCost` for that mode. We avoid that complexity in v1
and only support cost-1 diagonal.

**Effort:** ~1.5 hours (most of it is tests).
**Owner:** `@core` + `@algorithm`.
**Risk:** Manhattan + 8-conn is inadmissible — must be guarded at the
CLI to prevent silent suboptimal results.

---

## 3. Octile heuristic

The heuristic for 8-conn with `sqrt(2)` diagonal cost. Derivation in
[`HEURISTICS.md`](HEURISTICS.md) §3.3 sidebar; full worked example in
[`HEURISTICS.md`](HEURISTICS.md) §5.

**Effort:** 30 minutes.
**Owner:** `@heuristic`.
**Risk:** zero.

---

## 4. Bidirectional A\*

Run A\* from start and from goal simultaneously; meet in the middle.
Cuts expansions by ~`sqrt(d)` factor on open maps.

**New surface:**
```cpp
class BidirectionalAStar : public IPathfinder {
public:
    explicit BidirectionalAStar(const IHeuristic& h);
    Result run(const Grid&, const RunConfig&,
               IVisualizer*, Metrics*) const override;
};
```

**Existing surfaces touched:** none (it's a sibling of `AStar`).
**Tests:** equivalence with `AStar` on optimal cost.
**Effort:** ~3 hours.
**Owner:** `@algorithm`.
**Risk:** Termination condition is subtle (best-`f` rule). Requires careful
write-up. [Reference: Pohl 1971; Goldberg & Werneck 2005.]

---

## 5. Jump Point Search (JPS)

Symmetry-aware A\* on uniform-cost grids. ~10× faster than vanilla A\*
on open maps; same optimality.

**Effort:** 6–8 hours (the algorithm is one paper plus careful state
machines).
**Owner:** `@algorithm`.
**Risk:** Pulls in a uniform-cost assumption (no weights). Not a
universal replacement.

---

## 6. Theta\* (any-angle pathfinding)

Smooths the A\* path by allowing line-of-sight shortcuts. Useful when
movement is continuous, not grid-locked.

**Surfaces:** new `IPathfinder` subclass + a `LineOfSight` helper.
Reuses `Euclidean` heuristic.
**Effort:** ~6 hours.
**Owner:** `@algorithm`.
**Risk:** Output is no longer a sequence of cell coordinates; we need to
generalise `Result.path` (or add a sibling `Result::waypoints`).

---

## 7. Heuristic tuning (`weight * h`)

Scaling A\*'s heuristic by `w > 1` makes the search faster but
inadmissible (returns paths up to `w` × optimal). A research lever, not
a v1 default.

**Surface:** `RunConfig::heuristicWeight = 1.0`.
**Effort:** 30 minutes.
**Owner:** `@algorithm`.
**Risk:** Easy to misuse. Must be loud in the CLI: prints
`(inadmissible: heuristic weight = 1.50)` next to the result.

---

## 8. Random maze generators

`pae --gen maze --width 100 --height 100 --seed 42 > out.txt`.

**Algorithm options:**
- Recursive backtracker (DFS) — sparse mazes.
- Prim's — open mazes.
- Eller's — fast, low-memory.

**Surface:** `pae::gen::IMazeGen`, classes per algorithm; CLI subcommand.
**Effort:** ~3 hours.
**Owner:** `@core`.
**Risk:** zero. Useful for benchmark map diversity.

---

## 9. GUI visualizer (SDL2 / SFML / Raylib)

Render the search live in a real window: pan/zoom, frame stepping,
side-by-side algorithm comparison.

**Surface:** new `pae_visualization_gui` library + `IVisualizer`
subclass. Selected via `--visualize gui`.
**External dep:** Raylib (smallest, MIT, single binary).
**Effort:** ~12 hours.
**Owner:** `@viz`.
**Risk:** First external dep that isn't header-only. Pushes v1 well
beyond the 12-hour budget. Hence: deferred.

---

## 10. Curses TUI visualizer

Interactive terminal UI: arrow keys to scrub through frames, panel
showing current `Metrics`.

**Surface:** new `IVisualizer` subclass. ncurses or
[`notcurses`](https://nick-black.com/dankwiki/index.php/Notcurses).
**Effort:** ~6 hours.
**Owner:** `@viz`.
**Risk:** ncurses is platform-dependent (Windows fallback via PDCurses
or notcurses-windows); doubles CI surface.

---

## 11. JSON / YAML map format

Beyond the ASCII text format. Useful for round-tripping with editors
and machine generation.

**Surface:** alternate `MapLoader` strategy; `--map-format yaml`.
**External dep:** `nlohmann::json` (MIT, header-only) — only if
selected.
**Effort:** ~2 hours.
**Owner:** `@core`.
**Risk:** Drift between formats — must keep one canonical schema.

---

## 12. Memory profiling — real RSS, not analytical

Wire in `getrusage` (POSIX) / `GetProcessMemoryInfo` (Windows) for
true peak resident set.

**Surface:** `Metrics::trueRssBytes` field; only populated when
`-DPAE_TRUE_RSS=ON`.
**Effort:** ~1 hour.
**Owner:** `@perf`.
**Risk:** OS-noisy; measurements vary by allocator. Useful for
deep-dive, not for CI gating.

---

## 13. Thread-parallel benchmark sweep

Currently the sweep is sequential. With one algorithm-run per
worker thread, we'd parallelise the benchmark, not the search itself.

**Surface:** `Benchmark::Config::threads`.
**Effort:** ~2 hours; needs ThreadSanitizer in CI.
**Owner:** `@perf`.
**Risk:** Algorithms must remain trivially `const`. They are; this
remains a future-clean change.

---

## 14. Logging framework

Right now we use `std::cerr` for the App layer and silent algorithms.
A real `pae::log::Logger` with levels (`trace`, `info`, `warn`,
`error`), per-module sinks, and `--log-level` would help debugging
larger maps.

**Surface:** new `pae_log` library; injected into each library at
construction.
**Effort:** ~3 hours.
**Owner:** `@core`.
**Risk:** Adding a logger to hot loops kills perf. Must `noexcept`-
inline a no-op when level is below threshold (compile-time, ideally).

---

## 15. Replanning algorithms (D\*, LPA\*, ARA\*)

Useful when the grid changes mid-run. Out of v1 scope and out of v1.x
scope. Tracked here for completeness.

**Effort:** 20+ hours each. **Owner:** `@algorithm`. **Risk:** breaks
the "Grid is immutable" invariant; would require a substantial LLD
revision.

---

## Licensing notes (cross-cutting)

If we adopt any external dep beyond Catch2:

| Dep | License | Compatible with MIT? |
|-----|---------|---------------------|
| Google Benchmark | Apache-2.0 | yes |
| Raylib | zlib/libpng | yes |
| nlohmann/json | MIT | yes |
| SDL2 | zlib | yes |
| ncurses | MIT-like (BSD-with-attribution) | yes |
| notcurses | Apache-2.0 | yes |

Pinning rules in [`SECURITY.md`](../SECURITY.md) §"Supply chain
pinning" apply. Any new dep is a **PR with a `dep:` scope** and a
SECURITY.md update.
