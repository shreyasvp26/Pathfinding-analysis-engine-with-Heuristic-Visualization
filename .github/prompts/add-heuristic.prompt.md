---
mode: 'agent'
description: 'Add a new IHeuristic end-to-end: header → impl → registry → tests → docs.'
---

Add a new heuristic:

${input:heuristicName:Class name in CamelCase — e.g. "Octile"}
${input:lowerName:Lowercase CLI name — e.g. "octile"}
${input:movementModel:Movement model where this heuristic is admissible — e.g. "8-conn, diagonal cost sqrt(2)"}
${input:formula:The h(a,b) formula — e.g. "max(dx,dy) + (sqrt(2)-1)*min(dx,dy)"}

## Phase 1 — Plan

1. Confirm `${heuristicName}` is **admissible** for `${movementModel}`.
   Sketch the proof in the PR description.
2. Add `F-XXX` row to `docs/FEATURES.md`, owner `@heuristic`.
3. Branch: `git checkout -b feature/F<id>-${lowerName}`.

## Phase 2 — Implement (`@heuristic`)

1. Header: `pae/include/pae/heuristics/${heuristicName}.hpp`.
   ```cpp
   #pragma once
   #include "pae/heuristics/IHeuristic.hpp"

   namespace pae::heur {
   class ${heuristicName} : public IHeuristic {
   public:
       double estimate(core::Coord a, core::Coord b) const noexcept override;
       std::string_view name() const noexcept override { return "${lowerName}"; }
   };
   }
   ```
2. Impl: `pae/src/heuristics/${heuristicName}.cpp` implementing
   `${formula}` with `noexcept` and no allocations.

## Phase 3 — Register (`@core`)

```cpp
Registry<IHeuristic>::instance().reg(
    "${lowerName}",
    [] { return std::make_unique<heur::${heuristicName}>(); });
```

## Phase 4 — Test (`@qa`)

In `pae/tests/test_${lowerName}.cpp`:

- Property battery (non-negative, symmetric, goal-zero).
- Sampled admissibility against true cost in `${movementModel}`.
- A\* with `${heuristicName}` on `pae/maps/maze_50x50.txt` produces the
  same `totalCost` as A\* with another admissible heuristic for the
  same model (or as Dijkstra).

## Phase 5 — Document

1. Append §3.x to `docs/HEURISTICS.md` (model table + when-to-use).
2. Update the decision matrix in `docs/HEURISTICS.md` §4.
3. Add benchmark row in `docs/HEURISTICS.md` §7 (after running).
4. `docs/FEATURES.md` row → `Completed`.
5. `docs/CHANGELOG.md`: `feat(heur): add ${heuristicName} ([#PR])`.

## Phase 6 — Ship

Conventional commit: `feat(heur): add ${heuristicName}`. PR via
template. Reviewers `@heuristic` + `@qa`.

**Nothing in `AStar`, `IHeuristic`, or anything else is modified.**
You add a class, you add a registration line, you add a test file, you
update docs. Done.
