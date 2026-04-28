---
mode: 'agent'
description: 'Complete feature lifecycle: implement → test → verify → docs → ship. One command does everything.'
---

Ship this feature end-to-end:

${input:featureDescription:What to build — e.g. "Add the Octile heuristic for 8-conn diagonal-cost-sqrt(2) grids"}

**This is a composite workflow. Chain ALL phases — do not stop after
implementation.**

## Phase 1 — Plan

1. Read `docs/FEATURES.md` — find or create the matching `F-XXX` row.
2. Read the relevant deep-dive: `docs/LLD.md`, `docs/ALGORITHMS.md`,
   `docs/HEURISTICS.md`, `docs/DATA_STRUCTURES.md`, depending on the
   feature.
3. Search GitHub Issues for the feature; either work on the existing
   one or open a new one with the **Feature request** template.
4. Auto-detect the owning agent (see [`AGENTS.md`](../../AGENTS.md)
   §"File ownership rules"). The change must touch only that agent's
   roots.
5. Branch: `git checkout -b feature/F<id>-short-name`.

## Phase 2 — Implement

1. Add the new header in `pae/include/pae/<area>/`.
2. Add the implementation in `pae/src/<area>/`.
3. If a registry entry is needed, ask `@core` (or, if you ARE `@core`,
   add it to `pae/src/factory/register_default.cpp`).
4. Adhere to the auto-applied instructions:
   `.github/instructions/cpp-style.instructions.md` and the
   per-area instructions.

## Phase 3 — Test

1. Add a test file `pae/tests/test_<area>.cpp` (or extend an existing
   one). Cover EC-01 → EC-08 patterns from `docs/TESTING.md` §3 plus
   any feature-specific cases.
2. If the feature is algorithmic, add the cross-equivalence assertion
   (see `docs/TESTING.md` §4.2).
3. Run: `ctest --test-dir pae/build --output-on-failure`.

## Phase 4 — Verify (no skipping)

1. `cmake --build pae/build -j` — zero warnings, zero errors.
2. `cmake --build pae/build-rel -j` — zero warnings, zero errors.
3. ASan + UBSan: `ctest --test-dir pae/build` (with
   `-DPAE_SANITIZERS=ON`) — zero issues.
4. `clang-format --dry-run --Werror` on every changed file.
5. `clang-tidy -p pae/build` on every changed `.cpp`.
6. If perf-relevant: capture a benchmark diff (see
   `docs/PERFORMANCE.md` §10) and paste it into the PR description.
7. Cross-algorithm equivalence:
   `ctest --test-dir pae/build -R cross_algorithm`.

## Phase 5 — Document

1. Update `docs/FEATURES.md` — move row `Pending` → `Completed`.
2. Update `docs/CHANGELOG.md` — add a line under `[Unreleased]`.
3. Update the relevant deep-dive doc:
   - new algorithm → `docs/ALGORITHMS.md`,
   - new heuristic → `docs/HEURISTICS.md`,
   - new visualizer mode → `docs/REQUIREMENTS.md` §FR-4 + the
     visualization section of `LLD.md`.
4. If folder layout changed: `docs/FOLDER_STRUCTURE.md`.

## Phase 6 — Ship

1. Conventional commit: `feat(<scope>): <description>`.
2. Open PR using the `.github/PULL_REQUEST_TEMPLATE.md`.
3. The owning agent and `@qa` review.
4. CI must be green; squash-merge.

**All 6 phases happen automatically. The user only provides the
feature description.**
