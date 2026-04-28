# Pull request

## Summary

<!-- One paragraph: what changed and why. Link the issue this closes. -->

Closes #

## Type of change

- [ ] feat — new feature (no API break)
- [ ] fix — bug fix
- [ ] refactor — behaviour-preserving restructure
- [ ] perf — performance improvement
- [ ] docs — docs only
- [ ] test — tests only
- [ ] build / ci / chore

## Scope

<!-- Mark exactly the modules touched. -->

- [ ] core (`pae/{include,src}/pae/core/`)
- [ ] io (`pae/{include,src}/pae/io/`)
- [ ] heuristics (`pae/{include,src}/pae/heuristics/`)
- [ ] algorithms (`pae/{include,src}/pae/algorithms/`)
- [ ] visualization (`pae/{include,src}/pae/visualization/`)
- [ ] metrics (`pae/{include,src}/pae/metrics/`)
- [ ] factory (`pae/{include,src}/pae/factory/`)
- [ ] cli (`pae/src/cli/`)
- [ ] tests (`pae/tests/`)
- [ ] benchmarks (`pae/benchmarks/`)
- [ ] docs (`docs/`)
- [ ] build (`CMakeLists.txt`, scripts)
- [ ] ci (`.github/workflows/`)

## Owning agent

<!-- See AGENTS.md. -->

- [ ] `@core`
- [ ] `@algorithm`
- [ ] `@heuristic`
- [ ] `@viz`
- [ ] `@perf`
- [ ] `@qa`
- [ ] `@build`

## Checklist

- [ ] Code compiles with `-Wall -Wextra -Werror -Wpedantic` (or `/W4 /WX`).
- [ ] `ctest --test-dir pae/build` is green (Debug, Release).
- [ ] `clang-format --dry-run --Werror` clean.
- [ ] `clang-tidy` reports no new errors.
- [ ] ASan + UBSan clean (Debug job).
- [ ] If algorithmic: a Catch2 test verifies a hand-computed reference.
- [ ] If perf-relevant: benchmark numbers are pasted below.
- [ ] `docs/CHANGELOG.md` `[Unreleased]` updated.
- [ ] `docs/FEATURES.md` row updated.
- [ ] `docs/BUGS.md` row moved to `Fixed` if this closes a bug.
- [ ] If a new contract is introduced: matching deep-dive doc updated
      (`ALGORITHMS.md` / `HEURISTICS.md` / `DATA_STRUCTURES.md`).

## Performance impact (if relevant)

<!--
Run before/after the change:
  ./pae/build-rel/benchmarks/pae_bench --json > before.json
  ./pae/build-rel/benchmarks/pae_bench --json > after.json
  python pae/scripts/perf_diff.py before.json after.json

Paste the diff here. >5% regression = blocker unless BUDGET_WAIVER below.
-->

```text
(perf diff goes here)
```

`BUDGET_WAIVER:` <!-- only if you accept a >5% regression; explain why -->

## Deferred

<!-- Anything explicitly out of scope for this PR. Each item should
     reference the agent who owns it. -->

- Deferred to `@<agent>`: …
