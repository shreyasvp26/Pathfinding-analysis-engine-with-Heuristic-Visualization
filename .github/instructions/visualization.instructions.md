---
description: 'Auto-applies when editing the CLI visualizer.'
applyTo: 'pae/**/visualization/**'
---

# Visualization — coding rules

## Source of truth

- Class shapes: `docs/LLD.md` §5.
- Snapshot tests: `docs/TESTING.md` §5.
- Requirements: `docs/REQUIREMENTS.md` §FR-4.

## The contract

```cpp
class IVisualizer {
public:
    virtual ~IVisualizer() = default;
    virtual void onSearchStart(const Grid& g)             = 0;
    virtual void onEnqueue(Coord c, double f)             = 0;
    virtual void onExpand(Coord c)                        = 0;
    virtual void onPathFound(std::span<const Coord> p)    = 0;
    virtual void onSearchComplete(const Metrics& m)       = 0;
};
```

The interface is set. Adding a method requires a coordinated PR
touching every concrete visualizer + every algorithm.

## Modes

`enum class VizMode { None, Final, Step };`

- **None** — `NullVisualizer`. No-op. Used in benchmarks.
- **Final** — render once, on `onPathFound`.
- **Step** — render on each `onExpand`, throttled by `fpsCap`.

## Hard rules

1. **Algorithms know nothing about ANSI codes.** They call `onExpand`.
   The visualizer renders.
2. **Throttle in `Step` mode.** Default `fpsCap = 30`. Compute
   `auto now = clock::now(); if (now - lastFrame_ < period) return;`.
3. **`--no-color`** disables ANSI escapes. Snapshot tests run in
   no-color mode.
4. **Reset on `onSearchStart`.** Two consecutive runs in the same
   process must produce identical output.
5. **No allocations per event.** The internal buffers (`openSet_`,
   `closedSet_`) are `std::vector<uint8_t>` sized once in
   `onSearchStart`.

## Character mapping

| Cell type | Character | Color (when on) |
|-----------|----------|----------------|
| Empty | `.` | default |
| Obstacle | `#` | gray |
| Start | `S` | bold green |
| End | `E` | bold red |
| Open / frontier | `o` | yellow |
| Closed / visited | `*` | dim cyan |
| Final path | `+` | bold magenta |

ANSI codes in `pae/src/visualization/AnsiCodes.hpp` (private impl
header). `--no-color` short-circuits to plain characters.

## Snapshot test format

Snapshots are ASCII-only (`--no-color`). Trailing whitespace stripped.
LF line endings. No timestamps, no metrics, no banners.

Example `tiny.bfs.final.txt`:

```
S+++.
##+#.
..+++
.#.#E
```

(Where `+` traces the BFS path.)

## Anti-patterns

| Anti-pattern | Why it's wrong |
|--------------|---------------|
| Calling `printf` from inside the algorithm | Couples algorithm to terminal; breaks NullVisualizer; breaks benchmarks. |
| `std::cout` (global) instead of an injected `std::ostream&` | Untestable; can't capture output for snapshots. |
| Per-event allocation (e.g., `std::string` formatting on every onExpand) | Kills perf in `Step` mode on big grids. |
| Forgetting to reset state on `onSearchStart` | Subsequent runs have ghost frontiers. |
| Hard-coding ANSI codes inside class methods | Move to `AnsiCodes.hpp` so `--no-color` is a single switch. |
