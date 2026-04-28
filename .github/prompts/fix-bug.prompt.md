---
mode: 'agent'
description: 'Single bug fix + BUGS.md update + CHANGELOG entry.'
---

Fix bug:

${input:bugId:Bug ID — e.g. "B017" or "S004"}
${input:summary:One-sentence summary of what's broken}

## Steps

1. Find or create the row in `docs/BUGS.md` Open section.
2. Add a failing test in `pae/tests/test_<area>.cpp` that reproduces
   the bug.
3. Implement the fix in the smallest possible scope.
4. Run `ctest --test-dir pae/build` — your new test passes; all
   existing tests still pass.
5. Move the row in `docs/BUGS.md` from `Open` to `Fixed`.
6. Add `fix(<scope>): <summary> ([#PR])` to `docs/CHANGELOG.md`
   `[Unreleased]`.
7. Commit: `fix(<scope>): <summary>`.
8. PR; CI green; merge.
9. Update the `Commit` field in the `Fixed` row with the squash SHA.

For a multi-bug parallel run, use `/multi-bug-fix`.
For a complete fix lifecycle (PR template, deferred sub-tasks, etc.),
use `/ship-fix`.
