# Folder Structure — Pathfinding Analysis Engine

Detailed breakdown of every folder and its purpose. Mirrors the
`docs/FOLDER_STRUCTURE.md` from `ssm_calender` and `Jyotish AI`.

---

## Root

| Path | Purpose |
|------|---------|
| `README.md` | Project overview & quick start |
| `AGENTS.md` | Agent definitions + orchestration map |
| `SECURITY.md` | Threat model & security posture |
| `LICENSE` | MIT license |
| `CMakeLists.txt` | Top-level CMake — delegates to `pae/` |
| `.gitignore` | Build artefacts, IDE, secrets |
| `.gitattributes` | LF line endings, binary asset markers |
| `.editorconfig` | 4-space indent, LF, UTF-8 |
| `.clang-format` | LLVM-derived format profile |
| `.clang-tidy` | Lint profile (bugprone, modernize, performance, …) |
| `.pre-commit-config.yaml` | Pre-commit hooks (clang-format, large files, secrets) |

## `.github/` — CI/CD & Copilot config

| Path | Purpose |
|------|---------|
| `copilot-instructions.md` | Master Copilot rules (mandatory steps, code style) |
| `SETUP_GITHUB.md` | Onboarding guide for new contributors |
| `dependabot.yml` | Weekly dependency PRs |
| `PULL_REQUEST_TEMPLATE.md` | PR description template |
| `ISSUE_TEMPLATE/bug_report.md` | Bug ticket template |
| `ISSUE_TEMPLATE/feature_request.md` | Feature ticket template |
| `ISSUE_TEMPLATE/performance_issue.md` | Perf regression template |
| `agents/core.agent.md` | Core engine agent persona |
| `agents/algorithm.agent.md` | A\*/Dijkstra/BFS agent persona |
| `agents/heuristic.agent.md` | Heuristic agent persona |
| `agents/visualization.agent.md` | Viz agent persona |
| `agents/performance.agent.md` | Metrics & benchmark agent persona |
| `agents/qa.agent.md` | QA / verification agent persona |
| `agents/build.agent.md` | CMake / CI agent persona |
| `instructions/cpp-style.instructions.md` | Auto-applies to all C++ files |
| `instructions/algorithms.instructions.md` | Auto-applies to algorithms folder |
| `instructions/heuristics.instructions.md` | Auto-applies to heuristics folder |
| `instructions/visualization.instructions.md` | Auto-applies to viz folder |
| `instructions/performance.instructions.md` | Auto-applies to metrics + benchmarks |
| `instructions/testing.instructions.md` | Auto-applies to tests folder |
| `prompts/ship-feature.prompt.md` | End-to-end feature lifecycle |
| `prompts/ship-fix.prompt.md` | End-to-end bugfix lifecycle |
| `prompts/add-algorithm.prompt.md` | Add a new `IPathfinder` end-to-end |
| `prompts/add-heuristic.prompt.md` | Add a new `IHeuristic` end-to-end |
| `prompts/benchmark.prompt.md` | Run + record canonical benchmark |
| `prompts/write-tests.prompt.md` | Generate Catch2 tests for a module |
| `prompts/run-checks.prompt.md` | Sequential verification |
| `prompts/parallel-checks.prompt.md` | Parallel verification |
| `prompts/code-review.prompt.md` | AI code review |
| `prompts/refactor.prompt.md` | Behaviour-preserving restructure |
| `prompts/fix-bug.prompt.md` | Single bug fix + docs |
| `prompts/implement-feature.prompt.md` | Implement spec from FEATURES.md |
| `prompts/multi-bug-fix.prompt.md` | Parallel multi-bug fix |
| `prompts/git-release.prompt.md` | Tag + version + changelog |
| `prompts/hotfix.prompt.md` | Hotfix branch + release |
| `prompts/setup-new-collaborator.prompt.md` | New dev onboarding |
| `mcp/pae_mcp.py` | MCP server for tooling (run_ctest, list_open_bugs, …) |
| `workflows/ci.yml` | Build + ctest + sanitizers (matrix Linux/macOS/Windows) |
| `workflows/validate.yml` | Lightweight PR check (clang-tidy + format) |
| `workflows/auto-ticket.yml` | Auto-prefix issue titles with collaborator code |
| `workflows/release.yml` | Build & attach binaries to GitHub Releases |

## `.vscode/` — Editor config

| Path | Purpose |
|------|---------|
| `settings.json` | Clangd, format-on-save, MCP server registration |
| `tasks.json` | Configure / Build / Test / Bench / Format tasks |
| `launch.json` | LLDB / GDB debug targets |
| `extensions.json` | Recommended: clangd, CMake Tools, GitLens |

## `docs/` — Documentation

| Path | Purpose |
|------|---------|
| `ARCHITECTURE.md` | High-level system design |
| `LLD.md` | Low-level design (every class, every signature) |
| `FOLDER_STRUCTURE.md` | This file |
| `REQUIREMENTS.md` | FR/NFR/constraints with verification rules |
| `FEATURES.md` | Feature tracker (Pending → In Progress → Completed) |
| `BUGS.md` | Bug tracker (Open → Fixed) |
| `CHANGELOG.md` | SemVer changelog (`[Unreleased]` rolls into next tag) |
| `IMPLEMENTATION_PLAN.md` | Phased SDLC plan; vertical slices |
| `ALGORITHMS.md` | Per-algorithm deep dive (pseudo-code, complexity, optimality) |
| `HEURISTICS.md` | Per-heuristic deep dive (admissibility, when to use) |
| `DATA_STRUCTURES.md` | Container choices and trade-offs |
| `TESTING.md` | Test strategy, fixtures, property tests, sanitizers |
| `PERFORMANCE.md` | Benchmark plan, methodology, targets |
| `EXTENSIONS.md` | Optional but valuable extensions |
| `ROADMAP.md` | Long-horizon roadmap (v0.1 → v1.0+) |

## `pae/` — The C++ engine (the "app folder")

```
pae/
├── CMakeLists.txt                       — top-level engine CMake
├── include/pae/                         — public headers (the contract surface)
│   ├── core/
│   │   ├── Coord.hpp
│   │   ├── Cell.hpp
│   │   ├── Node.hpp
│   │   ├── Grid.hpp
│   │   └── Span.hpp                     — C++17 polyfill for std::span
│   ├── io/
│   │   ├── GridLoader.hpp
│   │   └── Errors.hpp
│   ├── heuristics/
│   │   ├── IHeuristic.hpp
│   │   ├── Manhattan.hpp
│   │   ├── Euclidean.hpp
│   │   └── Chebyshev.hpp
│   ├── algorithms/
│   │   ├── IPathfinder.hpp
│   │   ├── RunConfig.hpp
│   │   ├── Result.hpp
│   │   ├── AStar.hpp
│   │   ├── Dijkstra.hpp
│   │   └── BFS.hpp
│   ├── visualization/
│   │   ├── IVisualizer.hpp
│   │   ├── VizMode.hpp
│   │   ├── CliVisualizer.hpp
│   │   └── NullVisualizer.hpp
│   ├── metrics/
│   │   ├── Metrics.hpp
│   │   ├── Benchmark.hpp
│   │   └── Report.hpp
│   ├── factory/
│   │   └── Registry.hpp
│   └── cli/
│       ├── AppConfig.hpp
│       └── App.hpp
│
├── src/                                 — implementations (private to library)
│   ├── core/Grid.cpp
│   ├── io/GridLoader.cpp
│   ├── heuristics/Manhattan.cpp
│   ├── heuristics/Euclidean.cpp
│   ├── heuristics/Chebyshev.cpp
│   ├── algorithms/AStar.cpp
│   ├── algorithms/Dijkstra.cpp
│   ├── algorithms/BFS.cpp
│   ├── visualization/CliVisualizer.cpp
│   ├── metrics/Benchmark.cpp
│   ├── metrics/Report.cpp
│   ├── factory/register_default.cpp     — registers built-ins (composition root)
│   ├── cli/AppConfig.cpp
│   ├── cli/App.cpp
│   └── cli/main.cpp                     — 5-line main()
│
├── tests/                               — Catch2 unit + property tests
│   ├── CMakeLists.txt
│   ├── test_grid.cpp
│   ├── test_grid_loader.cpp
│   ├── test_manhattan.cpp
│   ├── test_euclidean.cpp
│   ├── test_chebyshev.cpp
│   ├── test_astar.cpp
│   ├── test_dijkstra.cpp
│   ├── test_bfs.cpp
│   ├── test_cross_algorithm.cpp         — A* vs Dijkstra equivalence
│   ├── test_visualizer.cpp
│   ├── test_metrics.cpp
│   ├── test_registry.cpp
│   └── fixtures/                        — small known-answer maps
│       ├── tiny.txt
│       ├── corridor.txt
│       ├── no_path.txt
│       └── weighted_small.txt
│
├── benchmarks/
│   ├── CMakeLists.txt
│   ├── bench_pathfinders.cpp            — algorithm × heuristic sweep
│   └── results/                         — CSV/JSON output (gitignored)
│
├── maps/                                — sample grids (committed)
│   ├── maze_20x20.txt
│   ├── maze_50x50.txt
│   ├── maze_100x100.txt
│   ├── open_arena_50x50.txt
│   ├── corridor.txt
│   └── weighted_small.txt
│
└── scripts/
    ├── format.sh                        — clang-format -i across the tree
    ├── format.ps1                       — Windows equivalent
    ├── run-checks.sh                    — configure + build + ctest + tidy
    ├── run-benchmarks.sh                — runs `pae --benchmark` over `maps/`
    └── bump-version.sh                  — SemVer bump in CMakeLists + CHANGELOG
```

## `design/` — Visual / planning assets (optional)

| Path | Purpose |
|------|---------|
| (placeholder) | UML diagrams, asciinema casts, sample CLI screenshots |

---

## Header vs. implementation separation

**Rule.** Anything visible to other libraries lives in
`pae/include/pae/<module>/`. Anything private to the implementation
lives in `pae/src/<module>/` — including helper headers if needed
(e.g., `pae/src/algorithms/internal/PriorityQueueAdapter.hpp`).

**Rule.** A `#include "pae/<module>/X.hpp"` is allowed across libraries
**only** if `X.hpp` lives in `pae/include/pae/<module>/`. CMake enforces
this via `target_include_directories(... PUBLIC include)` for headers
and `PRIVATE src` for impl-only headers.

---

## Naming conventions

| Kind | Convention | Example |
|------|-----------|---------|
| Namespaces | `lower_snake` | `pae::algo` |
| Classes / Structs | `CamelCase` | `Grid`, `AStar` |
| Interfaces | `I` prefix | `IPathfinder`, `IHeuristic` |
| Methods / Functions | `camelCase` | `estimate`, `inBounds` |
| Member variables | `camelCase_` (trailing underscore) | `cells_`, `width_` |
| Enums | `CamelCase` enum class; values `CamelCase` | `enum class Cell { Empty, Obstacle, Start, End }` |
| Constants | `UPPER_SNAKE` | `static constexpr int MAX_GRID_DIM = 1024;` |
| Files | Match the primary type, `CamelCase.hpp/.cpp` | `AStar.hpp`, `AStar.cpp` |
| Header guards | `#pragma once` | (no `#ifndef` guards) |

---

## Why this layout (and why mirror `ssm-app`)

1. **One agent per top-level module.** The folder map above corresponds
   1-to-1 to the agents in [`AGENTS.md`](../AGENTS.md). A QA-agent
   `cd pae/tests/` is a self-contained world; a heuristic-agent
   `cd pae/src/heuristics/` is too. Conflict-free parallel work.
2. **Public headers in `include/`, impl in `src/`.** Consistent with
   modern CMake idiom, integrates cleanly with `clangd` and IDEs.
3. **Per-module CMake target.** Each library is its own `add_library`,
   independently testable. Mirrors the SSM/Jyotish micro-module style
   (`chart_engine/`, `prediction_engine/`, `panchanga/`).
4. **`docs/` parallel to code, not nested under it.** This keeps
   documentation discoverable from the repo root, identical to
   `ssm_calender/docs/` and `Jyotish AI/docs/`.
