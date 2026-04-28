# Bugs — Pathfinding Analysis Engine

Two sections: `Open` and `Fixed`. New bugs go into Open. When a fix
ships, the row moves to Fixed with the merge commit SHA, the PR
number, and the version it shipped in. We never delete rows from
Fixed.

ID convention (mirrors `ssm_calender`'s ticket scheme):
- `B###` for engineering bugs
- letter prefix tied to GitHub username:
  - `S###` — opened by `shreyasvp`
  - (others added as collaborators join — see `.github/workflows/auto-ticket.yml`)

GitHub issues opened by these users get auto-prefixed by the
`auto-ticket` workflow.

Severity tags: `critical` (data loss / crash on default invocation),
`high` (incorrect output on a documented use case), `medium`
(incorrect output on an edge case or unhelpful error), `low` (cosmetic).

---

## Open

| ID | Severity | Title | Component | Reported | Notes |
|----|---------|-------|-----------|----------|-------|
|    |         | _(none yet — repo just initialised)_ |            |          |       |

---

## Fixed

| ID | Severity | Title | Fixed in | Commit | PR | Notes |
|----|---------|-------|----------|--------|----|-------|
|    |         | _(none yet)_ |   |        |    |       |

---

## How to file a bug

1. Open a GitHub Issue with the **Bug Report** template
   (`.github/ISSUE_TEMPLATE/bug_report.md`).
2. The auto-ticket workflow prefixes the title with your collaborator
   code (e.g., `S001: …`).
3. The bug appears in this file's `Open` section in the next PR that
   touches docs (or in the same PR that introduces the bug, if the
   reporter is also fixing it).
4. When the fix merges, the row moves to `Fixed` with:
   - `Fixed in`: target version (`v0.2.0`, `v1.0.0`, etc.)
   - `Commit`: the merge commit SHA (12 chars)
   - `PR`: GitHub PR number

## How to escalate

A `critical` bug:
- blocks the next release tag,
- gets a hotfix branch (`hotfix/Bxxx-short-name`),
- ships a patch version even if mid-phase.

A `high` bug:
- blocks a PR that touches the same component until a fix is in flight,
- is targeted at the **next** minor release.

A `medium` or `low` bug:
- ships when the owning agent picks it up; no schedule guarantee.
