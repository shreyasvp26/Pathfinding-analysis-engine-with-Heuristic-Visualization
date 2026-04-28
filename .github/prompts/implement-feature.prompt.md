---
mode: 'agent'
description: 'Implement a feature from a FEATURES.md row spec.'
---

Implement the feature:

${input:featureId:Feature ID — e.g. "F-203" (look it up in docs/FEATURES.md)}

## Steps

1. Read the row in `docs/FEATURES.md`. Identify the owning agent and
   the relevant deep-dive doc (LLD section, ALGORITHMS section, etc.).
2. Move the row's status from `Pending` to `In Progress`.
3. Branch: `git checkout -b feature/${featureId}-short-name`.
4. Implement following the deep-dive doc's contract exactly. **Do not
   invent signatures.**
5. Add tests covering EC-01 → EC-08 (algorithms) or the property
   battery (heuristics) or snapshots (viz) per
   `docs/TESTING.md` §3–§5.
6. Run `/parallel-checks` (or `/run-checks`).
7. Update `docs/FEATURES.md` row to `Completed`.
8. Append a line to `docs/CHANGELOG.md` `[Unreleased]`.
9. Commit (`feat(<scope>): …`); open PR using the standard template.

For a fully orchestrated lifecycle (issue tracking, perf diff,
multi-agent review), use `/ship-feature`.
