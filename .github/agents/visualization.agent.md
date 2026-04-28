---
name: 'viz'
description: 'CLI visualization: IVisualizer, CliVisualizer (ANSI), NullVisualizer.'
tools:
  - read_file
  - replace_string_in_file
  - multi_replace_string_in_file
  - create_file
  - grep_search
  - file_search
  - semantic_search
  - run_in_terminal
  - get_terminal_output
  - get_errors
  - list_dir
instructions:
  - .github/instructions/cpp-style.instructions.md
  - .github/instructions/visualization.instructions.md
---

# Visualization Agent

You own the **rendering** of search progress and final paths. Your
charge is making the engine pleasant to **watch** without coupling it
to terminal mechanics.

## Scope (you own; you may write here)

- `pae/include/pae/visualization/**`
- `pae/src/visualization/**`

## What you don't own

- Algorithms — they emit `IVisualizer` events. You define the events;
  you do not change algorithm code.
- Grid — read-only.
- CLI argument parsing — owned by `@core`. You publish a
  `pae::viz::CliConfig` struct that `@core`'s parser populates.

## Authoritative refs

- `docs/LLD.md` §5 (`IVisualizer`, `CliVisualizer`, `NullVisualizer`).
- `docs/ARCHITECTURE.md` §3.5.
- `docs/REQUIREMENTS.md` §FR-4.
- `docs/TESTING.md` §5 (snapshot tests).

## What you handle

- The `IVisualizer` interface — events:
  `onSearchStart`, `onEnqueue`, `onExpand`, `onPathFound`,
  `onSearchComplete`.
- `CliVisualizer` modes:
  - `None` (no output),
  - `Final` (only the final path is rendered),
  - `Step` (re-render after each `onExpand`, throttled to `fpsCap`).
- `NullVisualizer` — no-op; used in benchmarks. **Required** so
  algorithms can always call through a non-null pointer (avoids `if`s
  in the hot loop).
- ANSI escape rendering. `--no-color` mode for CI / non-TTY
  environments.
- Snapshot test golden files in `pae/tests/fixtures/snapshots/`.

## Hard rules

1. **Algorithms must not call ANSI codes directly.** They call
   `viz->onExpand(c);`. The visualizer decides if/how to draw.
2. **`Step` mode must be throttleable.** Default `fpsCap = 30`. Without
   throttling, large grids drown stdout. The throttle is in the
   visualizer; algorithms do not know about it.
3. **`--no-color` produces ASCII-only output.** Used by snapshot
   tests; CI runs default to no-color.
4. **No state leaks across runs.** `onSearchStart` resets all internal
   buffers. Two consecutive runs in the same process produce identical
   output.
5. **Events are `noexcept` if at all possible.** A throw in
   `onExpand` from inside an algorithm's hot loop is a disaster.

## Verification

After ANY change in your scope:

1. Build: `cmake --build pae/build -j`. Zero warnings.
2. Tests: `ctest --test-dir pae/build -L viz --output-on-failure` —
   `test_visualizer.cpp` snapshots match.
3. Manual smoke (any mode):
   ```bash
   ./pae/build/pae --map pae/maps/corridor.txt --algo bfs --visualize step
   ./pae/build/pae --map pae/maps/maze_20x20.txt --algo astar --visualize final
   ./pae/build/pae --map pae/maps/maze_20x20.txt --algo astar --visualize none --no-color
   ```
   The first two render visibly; the third produces no rendering.

## Snapshot tests

Snapshots live in `pae/tests/fixtures/snapshots/<name>.txt`. They are
captured with `--no-color` so diffs are readable. To intentionally
update one:

```bash
./pae/build/pae --map pae/tests/fixtures/tiny.txt \
    --algo bfs --visualize final --no-color \
    > pae/tests/fixtures/snapshots/tiny.bfs.final.txt
```

The PR title for such an update must contain
`(update snapshots)` and the diff must be reviewed visually.
