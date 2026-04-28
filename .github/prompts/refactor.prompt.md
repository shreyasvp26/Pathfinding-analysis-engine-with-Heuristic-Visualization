---
mode: 'agent'
description: 'Behaviour-preserving restructure. No semantic change; tests must remain identical.'
---

Refactor:

${input:target:What to restructure — e.g. "Extract neighbour iteration in algorithms into a shared helper"}

## Hard rule

A refactor is **behaviour-preserving**. Test outputs, benchmarks
(within ±2%), and CLI behaviour must be byte-identical to before.

## Steps

1. Read `docs/LLD.md` and the affected file(s).
2. Capture the current state:
   - `ctest --test-dir pae/build --output-on-failure > /tmp/before.txt`
   - `./pae/build-rel/benchmarks/pae_bench --json > /tmp/bench.before.json`
3. Make the change. Move code, rename, extract, inline, etc. Do NOT
   change algorithmic behaviour.
4. Re-run:
   - `ctest --test-dir pae/build --output-on-failure > /tmp/after.txt`
   - `diff /tmp/before.txt /tmp/after.txt` should differ only in
     timing lines and test order, not in pass/fail.
   - `python pae/scripts/perf_diff.py /tmp/bench.before.json /tmp/bench.after.json`
     should show all `wallMicros.median` deltas within ±2%.

## Documentation

If the refactor moved files or renamed types, update:
- `docs/FOLDER_STRUCTURE.md` if structure changed,
- `docs/LLD.md` if names/signatures changed,
- `docs/CHANGELOG.md`: `refactor(<scope>): <description>`.

## Anti-patterns to avoid

- "While I'm here, I also …" — pure refactor PRs only. Bundle nothing
  else.
- Changing tie-breaking, even "harmlessly". Tests will fail; if they
  don't, the test suite is wrong, not the refactor.
- Hiding behind "no observable change" when nodesExpanded changes.
  That IS observable.
