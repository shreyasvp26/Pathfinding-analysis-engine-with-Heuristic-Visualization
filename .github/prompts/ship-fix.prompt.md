---
mode: 'agent'
description: 'Complete bugfix lifecycle: investigate → fix → verify → docs → ship.'
---

Fix this bug end-to-end:

${input:bugDescription:What is broken — e.g. "B017: A* returns non-optimal path on dense_obstacle.txt with diagonal=true and Manhattan"}

## Phase 1 — Triage

1. Read `docs/BUGS.md`. If the bug is already there, note its `B###`
   ID; if not, add it to the `Open` section with severity (see
   `docs/BUGS.md` "How to file a bug").
2. Identify the owning agent (`@core`, `@algorithm`, `@heuristic`,
   `@viz`, `@perf`, `@qa`, `@build`) using
   [`AGENTS.md`](../../AGENTS.md) §"File ownership rules".
3. Branch: `git checkout -b bugfix/B<id>-short-name`.

## Phase 2 — Reproduce (test-first)

1. Add a failing test in `pae/tests/test_<area>.cpp` that captures the
   bug. The test must currently FAIL.
2. Run `ctest --test-dir pae/build` and confirm the failure.

## Phase 3 — Fix

1. Make the smallest change in the owning module that flips the test
   to passing.
2. If the fix requires changing an interface owned by another agent,
   stop and file an issue. Do not cross boundaries.

## Phase 4 — Verify

1. `ctest --test-dir pae/build --output-on-failure` — your new test
   passes; **no existing test regresses**.
2. ASan + UBSan green.
3. `clang-format` and `clang-tidy` clean.
4. If the bug was performance-related: re-run the benchmark and confirm
   the metric returned to baseline; paste before/after in the PR.

## Phase 5 — Document

1. `docs/BUGS.md` — move the row from `Open` to `Fixed`. Fill in
   `Fixed in`, `Commit` (will be the merge SHA), `PR`.
2. `docs/CHANGELOG.md` — line under `[Unreleased]`:
   `fix(<scope>): <one-line description> ([#PR])`.
3. If the fix exposed a documentation gap (e.g., the LLD did not
   specify the case the bug exploits), update the doc.

## Phase 6 — Ship

1. Conventional commit: `fix(<scope>): <description>`.
2. PR using the standard template.
3. CI green; squash merge.
4. After merge, edit the `Fixed` row's `Commit` field with the squash
   SHA.

**All 6 phases happen automatically. The user only provides the bug
description (or B### ID).**
