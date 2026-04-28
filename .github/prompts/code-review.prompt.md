---
mode: 'agent'
description: 'AI code review focused on C++ idioms, OOP correctness, perf, and contract drift.'
---

Review the diff (current branch vs `main`) of this repo.

${input:focus:Optional focus area — e.g. "performance" or "OOP design" or "tests"}

## Scope of review

Read the diff and check **every** file against:

### 1. C++ idioms (`.github/instructions/cpp-style.instructions.md`)

- `#pragma once` (no `#ifndef`).
- No `using namespace` in headers.
- Includes ordered: matching → pae → stdlib → third-party.
- Member variables `camelCase_`.
- `[[nodiscard]]` on result-bearing functions.
- `noexcept` only when the contract is real.
- No raw `new`/`delete`. No C-style casts. No `std::endl`.

### 2. OOP / contracts (`docs/LLD.md`, `docs/ARCHITECTURE.md` §10)

- Algorithms are still `IPathfinder`s (not subclasses of each other).
- Heuristics are still `IHeuristic`s.
- Visualizers are still `IVisualizer`s.
- File-ownership boundaries respected (the change touches only the
  agent's roots).
- No new dependencies between modules that violate the dependency
  graph (`docs/ARCHITECTURE.md` §5).

### 3. Performance (`docs/PERFORMANCE.md`, `docs/DATA_STRUCTURES.md`)

- No allocations in algorithm hot loops.
- No `std::function` in inner loops.
- No `unordered_set` for closed (use `gScore` sentinel).
- No `std::map` (use `unordered_map`, or better, an array).
- `priority_queue` reserved up front.

### 4. Tests (`docs/TESTING.md`)

- New behaviour has a new test.
- Tests are deterministic, fast, label-tagged.
- No `std::cout`, no `sleep_for`, no `random_device`.
- Cross-algorithm equivalence still holds.

### 5. Documentation drift

- If a contract changed, the matching deep-dive doc was updated.
- `docs/CHANGELOG.md` `[Unreleased]` has a new line.
- `docs/FEATURES.md` row state matches reality.

## Output

For each issue found, emit:

> **[severity]** _file:line_ — finding. _Suggested fix_: …

Severity: `blocker` (must fix before merge), `major` (should fix this
PR), `nit` (next time is fine).

End with a one-line verdict:

```
APPROVE / REQUEST CHANGES / COMMENT
```
