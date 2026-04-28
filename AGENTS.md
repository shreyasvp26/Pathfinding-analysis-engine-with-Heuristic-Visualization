# Pathfinding Analysis Engine — Agent Definitions

This file mirrors the agent-orchestration pattern used in
[`../ssm_calender/AGENTS.md`](../ssm_calender/AGENTS.md) and
[`../Jyotish AI/AGENTS.md`](../Jyotish%20AI/AGENTS.md).
Every agent is a **scoped specialist** with explicit ownership boundaries
and verification contracts. Cross-cutting work is coordinated through
prompts in [`.github/prompts/`](.github/prompts).

Primary spec: [`docs/IMPLEMENTATION_PLAN.md`](docs/IMPLEMENTATION_PLAN.md).
Architecture: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).
LLD: [`docs/LLD.md`](docs/LLD.md).

---

## Invocable agents (`.agent.md` files)

Use these in VS Code Copilot Chat (or any compatible runner) with
`@<name>`:

| Agent | Invoke | Scope | File |
|-------|--------|-------|------|
| Core | `@core` | Grid, Node, Coord, IO, factory/registry | [`.github/agents/core.agent.md`](.github/agents/core.agent.md) |
| Algorithm | `@algorithm` | A\*, Dijkstra, BFS implementations + `IPathfinder` interface | [`.github/agents/algorithm.agent.md`](.github/agents/algorithm.agent.md) |
| Heuristic | `@heuristic` | `IHeuristic` + Manhattan / Euclidean / Chebyshev | [`.github/agents/heuristic.agent.md`](.github/agents/heuristic.agent.md) |
| Visualization | `@viz` | `IVisualizer`, `CliVisualizer`, ANSI rendering | [`.github/agents/visualization.agent.md`](.github/agents/visualization.agent.md) |
| Performance | `@perf` | `Metrics`, `Benchmark`, comparison harness | [`.github/agents/performance.agent.md`](.github/agents/performance.agent.md) |
| QA | `@qa` | All verification (build / clang-tidy / tests / sanitizers) | [`.github/agents/qa.agent.md`](.github/agents/qa.agent.md) |
| Build | `@build` | CMake, CI workflows, packaging | [`.github/agents/build.agent.md`](.github/agents/build.agent.md) |

---

## File ownership rules

| Owner | Owns (write) | Read-only consumes |
|-------|--------------|--------------------|
| `@core` | `pae/include/pae/core/**`, `pae/src/core/**`, `pae/src/io/**`, `pae/include/pae/io/**`, `pae/include/pae/factory/**`, `pae/src/factory/**`, `pae/maps/**` | — |
| `@algorithm` | `pae/include/pae/algorithms/**`, `pae/src/algorithms/**` | core, heuristics, metrics |
| `@heuristic` | `pae/include/pae/heuristics/**`, `pae/src/heuristics/**` | core (`Coord`) |
| `@viz` | `pae/include/pae/visualization/**`, `pae/src/visualization/**`, `pae/src/cli/**` | core, algorithms (read traversal events) |
| `@perf` | `pae/include/pae/metrics/**`, `pae/src/metrics/**`, `pae/benchmarks/**` | algorithms |
| `@qa` | `pae/tests/**`, `.github/workflows/**` (verification jobs only) | everything (read-only) |
| `@build` | `pae/CMakeLists.txt`, `pae/**/CMakeLists.txt`, `.github/workflows/**`, `pae/scripts/**`, root build files | — |

**Rule:** an agent **must not** modify a file outside its owned roots.
If a change needs to cross a boundary, the orchestrator opens a sub-task
for the owning agent (see *Communication protocol* below).

---

## Communication protocol

Agents do not call each other directly. They coordinate through:

1. **Issues.** Every cross-agent dependency becomes a GitHub Issue with
   the `agent:<name>` label. The auto-ticket workflow assigns it a
   ticket number (see [`.github/workflows/auto-ticket.yml`](.github/workflows/auto-ticket.yml)).
2. **Interfaces.** All inter-module communication is defined in headers
   under `pae/include/pae/**`. An agent that needs a new method on an
   interface owned by another agent files an issue rather than editing
   the header itself.
3. **Trackers.**
   - [`docs/FEATURES.md`](docs/FEATURES.md) — the canonical state of every
     feature (Pending → In Progress → Completed).
   - [`docs/BUGS.md`](docs/BUGS.md) — open bugs (closed bugs move to a
     "Fixed" section with the commit SHA).
   - [`docs/CHANGELOG.md`](docs/CHANGELOG.md) — every merged PR adds a line
     under `[Unreleased]`.
4. **Daily handoff.** The end of every PR description must list any
   work that was deferred to another agent (`Deferred to @qa: …`).

---

## Reusable prompts

Type `/` in Copilot Chat to invoke these workflows. Full text under
[`.github/prompts/`](.github/prompts).

| Prompt | Command | Purpose |
|--------|---------|---------|
| Ship Feature | `/ship-feature` | Full lifecycle: implement → test → verify → docs |
| Ship Fix | `/ship-fix` | Investigate → fix → verify → docs |
| Add Algorithm | `/add-algorithm` | Add a new `IPathfinder` end-to-end (header → impl → tests → registry → docs) |
| Add Heuristic | `/add-heuristic` | Add a new `IHeuristic` end-to-end |
| Benchmark | `/benchmark` | Run cross-algorithm benchmark on the canonical map suite |
| Write Tests | `/write-tests` | Generate Catch2 tests for any module |
| Run Checks | `/run-checks` | Sequential verification suite (configure, build, ctest, sanitizers) |
| Parallel Checks | `/parallel-checks` | Same as `run-checks` but parallelised |
| Code Review | `/code-review` | AI review for C++ idioms, ownership, complexity |
| Refactor | `/refactor` | Restructure code preserving behaviour |
| Fix Bug | `/fix-bug` | Single bug + `BUGS.md` + `CHANGELOG` |
| Implement Feature | `/implement-feature` | Build a feature from `FEATURES.md` spec |
| Multi Bug Fix | `/multi-bug-fix` | Parallel investigation + fix of multiple bugs |
| Git Release | `/git-release` | Tag, changelog, version bump |
| Hotfix | `/hotfix` | Critical bug → hotfix branch → release |
| Setup Collaborator | `/setup-new-collaborator` | New dev onboarding |

---

## Auto-applied instructions

These load automatically when editing matching files (Copilot
`applyTo` glob).

| Instruction | Applies to | File |
|-------------|------------|------|
| C++ style | `pae/**/*.{hpp,cpp,h,cc}` | [`.github/instructions/cpp-style.instructions.md`](.github/instructions/cpp-style.instructions.md) |
| Algorithms | `pae/**/algorithms/**` | [`.github/instructions/algorithms.instructions.md`](.github/instructions/algorithms.instructions.md) |
| Heuristics | `pae/**/heuristics/**` | [`.github/instructions/heuristics.instructions.md`](.github/instructions/heuristics.instructions.md) |
| Visualization | `pae/**/visualization/**`, `pae/src/cli/**` | [`.github/instructions/visualization.instructions.md`](.github/instructions/visualization.instructions.md) |
| Performance | `pae/**/metrics/**`, `pae/benchmarks/**` | [`.github/instructions/performance.instructions.md`](.github/instructions/performance.instructions.md) |
| Testing | `pae/tests/**` | [`.github/instructions/testing.instructions.md`](.github/instructions/testing.instructions.md) |

---

## Orchestration patterns

### Add a new algorithm (e.g., Bidirectional A\*)

1. `/add-algorithm` — orchestrator prompt. It will:
   1. `@algorithm` adds header + impl + registry entry
   2. `@qa` writes Catch2 tests + parametric correctness check vs. Dijkstra
   3. `@perf` adds it to the benchmark sweep
   4. `@core` updates the CLI `--algo` choices
   5. Update `FEATURES.md` + `CHANGELOG.md`

### Add a new heuristic (e.g., Octile)

1. `/add-heuristic` — `@heuristic` adds class + admissibility test;
   `@perf` adds a row to the comparison table; docs updated.

### Pre-merge (fastest)

1. `/parallel-checks` or invoke `@qa` — runs configure, build (Debug+Release),
   ctest, clang-tidy, ASan/UBSan in parallel.

### Multi-file refactor

1. `/refactor` → restructure
2. `@qa` → verify nothing broke (zero ctest failures, zero clang-tidy regressions)
3. `@perf` → confirm no >5% perf regression on the canonical benchmark

---

## Agent persona summaries

### core
- **Owns:** `Grid`, `Node`, `Coord`, `GridLoader`, `Registry`.
- **Expertise:** value semantics in C++, container choice (vectors of
  `Node` vs. flat array indexing), file I/O, parsing, factory pattern.
- **Verification:** `ctest -L core`.

### algorithm
- **Owns:** `IPathfinder`, `AStar`, `Dijkstra`, `BFS`.
- **Expertise:** graph algorithms, priority queues, optimality proofs,
  invariants (closed set semantics, tie-breaking).
- **Verification:** `ctest -L algorithm` — must include "matches
  Dijkstra on uniform-weight maps" cross-check for every algorithm.

### heuristic
- **Owns:** `IHeuristic`, Manhattan, Euclidean, Chebyshev.
- **Expertise:** admissibility, consistency (monotone), connection to
  movement model (4-conn vs. 8-conn).
- **Verification:** `ctest -L heuristic` — admissibility property test
  + zero-distance test + symmetry test.

### visualization
- **Owns:** `IVisualizer`, `CliVisualizer`, ANSI escape rendering, CLI
  argument parsing.
- **Expertise:** terminal rendering, refresh rate, redraw-or-append
  trade-off, accessibility (no-color mode).
- **Verification:** `ctest -L viz` — golden-file snapshots of small
  grids; `--no-color` produces ASCII-only output.

### performance
- **Owns:** `Metrics`, `Benchmark`, comparison report generator.
- **Expertise:** wall-clock vs. monotonic clock, RDTSC pitfalls, memory
  high-water tracking, statistical reporting (median/p95).
- **Verification:** benchmark must be reproducible: same map + same
  algorithm + same heuristic → metrics within ±2% across 30 runs.

### qa
- **Owns:** `pae/tests/**`, verification workflows.
- **Expertise:** Catch2 v3, fixture grids, property-based testing,
  AddressSanitizer / UndefinedBehaviorSanitizer integration.
- **Verification:** runs the parallel-checks pipeline; reports a
  results table.

### build
- **Owns:** root and per-folder `CMakeLists.txt`, GitHub Actions
  workflows, `pae/scripts/*`.
- **Expertise:** modern CMake (target-based, `target_link_libraries`,
  `target_include_directories`), `FetchContent`, multi-config presets,
  CI matrix.
- **Verification:** clean build on Linux + macOS + Windows in CI.

---

## Daily workflow (SSM/Jyotish parity)

1. `git pull` — check `docs/BUGS.md` for open critical bugs.
2. Pick a task from `docs/FEATURES.md` or a labelled issue.
3. Branch: `feature/F<id>-short-name` or `bugfix/B<id>-short-name`.
4. Invoke the appropriate agent or prompt. Implement.
5. `cd pae && cmake --build build && ctest --test-dir build` — must pass.
6. `clang-tidy` + `clang-format --dry-run --Werror` — must pass.
7. Update `docs/FEATURES.md` (move row), `docs/CHANGELOG.md` (add line).
8. Open PR → CI green → merge.
