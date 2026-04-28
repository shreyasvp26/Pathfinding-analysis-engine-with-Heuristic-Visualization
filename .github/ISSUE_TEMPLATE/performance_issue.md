---
name: Performance issue
about: Engine output is correct but slower / heavier than expected.
title: "P-Xxx: <short title>"
labels: ["performance"]
assignees: []
---

## Workload

- Map: <!-- path or attached file -->
- Algorithm: <!-- astar / dijkstra / bfs -->
- Heuristic: <!-- manhattan / euclidean / chebyshev / none -->
- `--diagonal`: <!-- on / off -->
- Benchmark mode: <!-- yes / no -->

## Observed

```text
<paste `pae --benchmark --format json` output, or `Metrics` summary>
```

## Expected (from PERFORMANCE.md / NFR-2)

| Metric | Budget |
|--------|--------|
| wall_us median | |
| nodesExpanded | |
| approxPeakBytes | |

## Suspected cause

<!-- e.g. "Heuristic newly returning higher values triggers more node revisits."
     Reference the relevant section of DATA_STRUCTURES.md or ALGORITHMS.md. -->

## Reproduction (deterministic)

```bash
git checkout <commit-sha>
./pae/scripts/run-benchmarks.sh
```

## Environment

- CI runner type / dev box CPU:
- Compiler + flags:
- pae version:
