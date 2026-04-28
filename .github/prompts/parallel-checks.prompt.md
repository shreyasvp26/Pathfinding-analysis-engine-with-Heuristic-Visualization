---
mode: 'agent'
description: 'Run all verification checks in parallel via runSubagent. Fastest pre-merge gate.'
---

Run all verification checks in **parallel**. Use `runSubagent` to
spawn one agent per independent check. Aggregate results when all
finish.

## Parallel checks

Spawn each as an independent subagent (don't wait):

| ID | Subagent | Command |
|----|----------|---------|
| 1 | configure-debug | `cmake -S . -B pae/build -DCMAKE_BUILD_TYPE=Debug -DPAE_SANITIZERS=ON` |
| 2 | configure-release | `cmake -S . -B pae/build-rel -DCMAKE_BUILD_TYPE=Release` |

Then, when 1 and 2 have finished, in parallel:

| ID | Subagent | Command |
|----|----------|---------|
| 3 | build-debug | `cmake --build pae/build -j` |
| 4 | build-release | `cmake --build pae/build-rel -j` |

Then, when 3 and 4 have finished, in parallel:

| ID | Subagent | Command |
|----|----------|---------|
| 5 | ctest-debug | `ctest --test-dir pae/build --output-on-failure` |
| 6 | ctest-release | `ctest --test-dir pae/build-rel --output-on-failure` |
| 7 | clang-format | `clang-format --dry-run --Werror $(git ls-files 'pae/**/*.hpp' 'pae/**/*.cpp')` |
| 8 | clang-tidy | `clang-tidy -p pae/build $(git ls-files 'pae/src/**/*.cpp')` |

## Report

When all subagents finish, present:

| Check | Status | Details |
|-------|--------|---------|
| Configure (Debug) | ✓/✗ | |
| Configure (Release) | ✓/✗ | |
| Build (Debug) | ✓/✗ | warnings: N |
| Build (Release) | ✓/✗ | warnings: N |
| ctest (Debug+Sanitizers) | ✓/✗ | passed: N, failed: N, ASan: M, UBSan: K |
| ctest (Release) | ✓/✗ | passed: N, failed: N |
| clang-format | ✓/✗ | files needing format: N |
| clang-tidy | ✓/✗ | new errors: N |

If any row is ✗, surface the relevant excerpt and suggest the agent who
should fix it (per `AGENTS.md` §"File ownership rules"). Do not merge
until all rows are ✓.
