---
mode: 'agent'
description: 'Parallel investigation + fix of multiple bugs. Spawns one subagent per bug.'
---

Fix multiple bugs in parallel.

${input:bugIds:Comma-separated bug IDs — e.g. "B017,B018,B019"}

## Strategy

For each bug ID:

1. Look it up in `docs/BUGS.md` Open section.
2. Map it to the owning agent (per `AGENTS.md` §"File ownership rules"):
   - bug in algorithms → `@algorithm`
   - bug in heuristics → `@heuristic`
   - bug in core/io/cli → `@core`
   - bug in viz → `@viz`
   - bug in metrics/bench → `@perf`
   - bug in build/CI → `@build`
3. Spawn a subagent for that bug, scoped to the matching agent
   persona, with task:
   - Add failing test → fix → run `/parallel-checks` → update
     `docs/BUGS.md` and `docs/CHANGELOG.md`.

All subagents run **independently**, each on its own bugfix branch
(`bugfix/B<id>-short-name`).

## Aggregation

When all subagents finish, summarise in a table:

| Bug | Owner agent | Branch | Status |
|-----|-------------|--------|--------|
| B017 | @algorithm | bugfix/B017-... | ✓ green |
| B018 | @core | bugfix/B018-... | ✗ blocker — see below |
| ... | ... | ... | ... |

If any bug is blocked or escalates, **stop** and surface the cause to
the user. Do not merge a bugfix that breaks another.

## Merge order

Merge in dependency order: `@core` first (others may depend on its
fixes), then `@algorithm`, then `@heuristic`, then `@viz`/`@perf`,
then `@build`. Each merge triggers CI; subsequent branches must be
rebased on `main` before merge.
