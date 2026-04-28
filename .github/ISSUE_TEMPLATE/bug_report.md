---
name: Bug report
about: Something is broken or incorrect.
title: "B0xx: <short description>"
labels: ["bug"]
assignees: []
---

## Severity

- [ ] critical — crashes / data loss on default invocation
- [ ] high — wrong output on a documented use case
- [ ] medium — wrong output on an edge case, or unhelpful error
- [ ] low — cosmetic / minor

## Component

- [ ] core
- [ ] io
- [ ] heuristics
- [ ] algorithms
- [ ] visualization
- [ ] metrics
- [ ] factory
- [ ] cli
- [ ] build / ci / docs

## Reproduction

```bash
# Exact command(s)
./pae --map ... --algo ... --heuristic ...
```

If a map is required, attach the `.txt` file or paste it inline.

## Expected behaviour

<!-- e.g. "Path cost should be 14 (manually computed)." -->

## Actual behaviour

<!-- Paste output. -->

## Environment

- OS:
- CPU arch:
- Compiler + version:
- CMake version:
- `pae --version`:

## Additional context

<!-- Stack traces, screenshots, related PR/issue links. -->
