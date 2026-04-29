# Copilot / AI Instructions — Pathfinding Analysis Engine

Structured like
[`../ssm_calender/.github/copilot-instructions.md`](../../ssm_calender/.github/copilot-instructions.md)
and
[`../Jyotish AI/.github/copilot-instructions.md`](../../Jyotish%20AI/.github/copilot-instructions.md):
mandatory rules, file map, workflows, escalation.

---

## Mandatory

1. **Spec first.** [`docs/REQUIREMENTS.md`](../docs/REQUIREMENTS.md),
   [`docs/ARCHITECTURE.md`](../docs/ARCHITECTURE.md), and
   [`docs/LLD.md`](../docs/LLD.md) are authoritative. The matching
   algorithm/heuristic semantics live in
   [`docs/ALGORITHMS.md`](../docs/ALGORITHMS.md) and
   [`docs/HEURISTICS.md`](../docs/HEURISTICS.md). Do not invent class
   names, method signatures, or container choices not grounded there
   unless the user explicitly approves.
2. **OOP rules are not negotiable.** Every algorithm is an
   `IPathfinder`. Every heuristic is an `IHeuristic`. Every visualizer
   is an `IVisualizer`. New behaviour = new subclass + one factory
   line. **Existing classes are not modified** to "make room" for new
   features. (See [`docs/REQUIREMENTS.md`](../docs/REQUIREMENTS.md) §C-4.)
3. **Owner boundaries.** A change touches only files in the agent's
   ownership root (see [`AGENTS.md`](../AGENTS.md) §"File ownership
   rules"). Cross-boundary changes go through an issue + a separate PR
   from the owning agent.
4. **Determinism.** No `std::rand`, no `std::time`, no
   `std::random_device`. Tie-breaking is fully specified in
   [`docs/LLD.md`](../docs/LLD.md) §2.3. Tests are deterministic;
   flaky tests are bugs (`docs/BUGS.md`), not retries.
5. **Documentation.** After meaningful work:
   `docs/CHANGELOG.md` (`[Unreleased]`),
   `docs/FEATURES.md` (move row),
   `docs/BUGS.md` (move bug to Fixed if applicable),
   plus the relevant deep-dive doc if a contract changed.
6. **No new dependencies without a PR scope `dep:`.** Catch2 is the
   only fetched dep; everything else is the C++17 standard library.
   Adding a dep requires a `SECURITY.md` update.
7. **Conventional commits.**
   `type(scope): description`.
   - Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`,
     `chore`, `perf`, `build`, `ci`, `dep`.
   - Scopes: `core`, `io`, `heur`, `algo`, `viz`, `metrics`,
     `factory`, `cli`, `tests`, `bench`, `docs`, `ci`, `build`.
8. **Secrets.** Never commit `.env`, `*.pem`, real user data, real API
   keys. There are no secrets in v1; if your PR adds one, stop and ask.

---

## C++ style (enforced)

- **C++17**. No compiler-specific extensions.
- `-Wall -Wextra -Werror -Wpedantic` (or MSVC `/W4 /WX`).
- `clang-format` with `.clang-format` (LLVM-derived). Pre-commit fixes.
- `clang-tidy` with `.clang-tidy`. New errors are PR-blockers.
- `#pragma once` (no `#ifndef` guards).
- Headers in `pae/include/pae/<module>/`; impl in `pae/src/<module>/`.
- `using namespace` is **forbidden in headers** and discouraged in
  `.cpp` (function-scope only if at all).
- Member variables `camelCase_` (trailing underscore).
- Classes/structs `CamelCase`; interfaces prefixed `I` (e.g.,
  `IPathfinder`).
- Path types: `std::filesystem::path`. Read-only strings:
  `std::string_view`. Owned strings: `std::string`.
- No raw `new`/`delete`. Use `std::make_unique`.
- No raw owning pointers in interfaces. `unique_ptr` for ownership;
  `T&` or `T*` for borrowed.
- No allocations in algorithm hot loops. Reserve up front.
- `noexcept` only when the contract is real (not "to make compiler
  happy").
- No `std::shared_ptr` unless ownership is genuinely shared (it isn't,
  in v1).

---

## Multi-phase work

When a task spans multiple V-phases or workstreams (see
[`docs/IMPLEMENTATION_PLAN.md`](../docs/IMPLEMENTATION_PLAN.md) §2):

1. Track each piece in `docs/FEATURES.md` and/or GitHub Issues (one
   issue per feature row, not per file).
2. Prefer **small PRs** per submodule. A PR larger than ~600 changed
   lines is a smell.
3. After each merge: update the row in `docs/FEATURES.md`, the matching
   line in `docs/CHANGELOG.md`, and any contract docs that drifted.

---

## Verification

| What | Command | When |
|------|--------|------|
| Configure (Debug + sanitizers) | `cmake --preset san` (or `cmake -S pae -B pae/build -DCMAKE_BUILD_TYPE=Debug -DPAE_SANITIZERS=ON`) | once per change of CMake files |
| Configure (Release) | `cmake --preset release` (or `cmake -S pae -B pae/build-rel -DCMAKE_BUILD_TYPE=Release`) | for benchmarks / perf budget |
| Build | `cmake --build --preset debug` (or `cmake --build pae/build -j`) | after every code change |
| Tests | `ctest --preset debug` (or `ctest --test-dir pae/build --output-on-failure`) | after every code change |
| Specific test label | `ctest --test-dir pae/build -L heuristic` | targeted |
| Perf-budget tests (opt-in) | `cmake --preset perf && cmake --build --preset perf && ctest --test-dir pae/build/perf -L perf --output-on-failure` | when `algo/`, `heur/`, `core/`, `metrics/` change |
| Format check | `clang-format --dry-run --Werror $(git ls-files '*.hpp' '*.cpp')` | before commit |
| Tidy check | `clang-tidy -p pae/build $(git ls-files 'pae/src/**/*.cpp')` | before commit |
| Bench (when perf-relevant) | `./pae/build/release/benchmarks/pae_bench --json > after.json && python pae/scripts/perf_diff.py before.json after.json` | when `algo/`, `heur/`, `core/`, `metrics/` change |

> **Configure path.** The root `CMakeLists.txt` simply does
> `add_subdirectory(pae)` so you must configure with `-S pae` (or the
> preset equivalents) to land binaries at the documented paths.
> `-S .` is rejected by CI since it produces a nested
> `pae/build-rel/pae/pae` layout that `release.yml` does not expect.

The `/parallel-checks` prompt
([`./prompts/parallel-checks.prompt.md`](prompts/parallel-checks.prompt.md))
is the canonical pre-merge gate.

---

## File map

| Area | Path |
|------|------|
| Architecture | `docs/ARCHITECTURE.md` |
| LLD | `docs/LLD.md` |
| Requirements | `docs/REQUIREMENTS.md` |
| Implementation plan / SDLC | `docs/IMPLEMENTATION_PLAN.md` |
| Algorithms deep dive | `docs/ALGORITHMS.md` |
| Heuristics deep dive | `docs/HEURISTICS.md` |
| Data structures | `docs/DATA_STRUCTURES.md` |
| Testing | `docs/TESTING.md` |
| Performance | `docs/PERFORMANCE.md` |
| Extensions | `docs/EXTENSIONS.md` |
| Roadmap | `docs/ROADMAP.md` |
| Folder structure | `docs/FOLDER_STRUCTURE.md` |
| Bugs | `docs/BUGS.md` |
| Features | `docs/FEATURES.md` |
| Changelog | `docs/CHANGELOG.md` |
| Security | `SECURITY.md` |
| CI | `.github/workflows/ci.yml`, `validate.yml`, `auto-ticket.yml`, `release.yml` |
| Prompts | `.github/prompts/` |
| Agents | `.github/agents/` |
| Instructions (auto-applied) | `.github/instructions/` |
| MCP | `.github/mcp/pae_mcp.py` |

---

## Orchestration (SSM/Jyotish parity)

| Goal | Prompt |
|------|--------|
| Ship a feature end-to-end | [`prompts/ship-feature.prompt.md`](prompts/ship-feature.prompt.md) |
| Add a new pathfinder algorithm | [`prompts/add-algorithm.prompt.md`](prompts/add-algorithm.prompt.md) |
| Add a new heuristic | [`prompts/add-heuristic.prompt.md`](prompts/add-heuristic.prompt.md) |
| Run the canonical benchmark | [`prompts/benchmark.prompt.md`](prompts/benchmark.prompt.md) |
| Fix a bug | [`prompts/fix-bug.prompt.md`](prompts/fix-bug.prompt.md) or [`ship-fix.prompt.md`](prompts/ship-fix.prompt.md) |
| Tests | [`prompts/write-tests.prompt.md`](prompts/write-tests.prompt.md) |
| Pre-merge checks | [`prompts/parallel-checks.prompt.md`](prompts/parallel-checks.prompt.md) |
| Code review | [`prompts/code-review.prompt.md`](prompts/code-review.prompt.md) |
| Refactor | [`prompts/refactor.prompt.md`](prompts/refactor.prompt.md) |
| Hotfix | [`prompts/hotfix.prompt.md`](prompts/hotfix.prompt.md) |
| Release | [`prompts/git-release.prompt.md`](prompts/git-release.prompt.md) |
| Onboard a new collaborator | [`prompts/setup-new-collaborator.prompt.md`](prompts/setup-new-collaborator.prompt.md) |

---

## When stuck

Ask the user and cite the ambiguous section of the LLD or the master
plan. Do not guess:
- Algorithmic correctness details (e.g., tie-breaking subtleties).
- Heuristic admissibility for a non-standard movement model.
- Container or memory-layout decisions ([`docs/DATA_STRUCTURES.md`](../docs/DATA_STRUCTURES.md)).
- Any introduction of a new external dependency.

Also do not guess at platform-specific code (Windows vs. POSIX). When
behaviour diverges, read the relevant `<filesystem>`/`<chrono>`
guarantees in cppreference and link to the page in your PR.
