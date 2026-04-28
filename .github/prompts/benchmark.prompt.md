---
mode: 'agent'
description: 'Run the canonical benchmark sweep, save JSON, compare to baseline, post diff.'
---

Run the canonical benchmark and report the result.

${input:context:What changed since the last run — e.g. "A* now uses index-based parent map" or "no code change, baseline refresh"}

## Steps

1. Build Release:

   ```bash
   cmake -S . -B pae/build-rel -DCMAKE_BUILD_TYPE=Release
   cmake --build pae/build-rel -j
   ```

2. Run the sweep:

   ```bash
   ./pae/build-rel/benchmarks/pae_bench \
       --maps pae/maps/maze_20x20.txt,pae/maps/maze_50x50.txt,pae/maps/maze_100x100.txt,pae/maps/weighted_100x100.txt,pae/maps/open_arena_50x50.txt \
       --reps 30 \
       --warmup 3 \
       --json > /tmp/pae_bench_after.json
   ```

3. Compare to the baseline (whichever runner class matches):

   ```bash
   python pae/scripts/perf_diff.py \
       pae/benchmarks/baselines/<runner>/sweep.json \
       /tmp/pae_bench_after.json
   ```

4. Format the output as a markdown table:
   - per `(map, algorithm, heuristic)`, show before / after / delta % for
     `wallMicros.median`, `nodesExpanded`, `approxPeakBytes`.
   - flag any row with `wallMicros` delta > +5% as ❌ (regression).
   - flag any row with `nodesExpanded` or `approxPeakBytes` delta != 0 as
     ⚠ (semantic change — must justify).
5. Paste the table into the PR description (or post as a comment).

## When to update the baseline

Update only via a deliberate PR titled
`bench: refresh baseline (<runner>, <reason>)`. Do **not** update the
baseline as a side-effect of feature work.

A baseline refresh PR includes:

- the new `pae/benchmarks/baselines/<runner>/sweep.json`,
- the runner type / kernel / CPU / compiler in
  `pae/benchmarks/baselines/<runner>/README.md`,
- a summary of why the baseline drifted (compiler upgrade, runner
  change, intentional algorithmic change).
