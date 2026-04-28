---
name: Feature request
about: Propose a new capability for the engine.
title: "F-Xxx: <short title>"
labels: ["enhancement"]
assignees: []
---

## What

<!-- One paragraph describing the feature. -->

## Why

<!-- The user / engineering need this addresses. -->

## Owning agent

<!-- See AGENTS.md. Pick exactly one. -->

- [ ] `@core`
- [ ] `@algorithm`
- [ ] `@heuristic`
- [ ] `@viz`
- [ ] `@perf`
- [ ] `@qa`
- [ ] `@build`

## Public surface (proposed)

<!-- Sketch the headers / interfaces / CLI flags this adds. -->

```cpp
// e.g.
// class Octile : public IHeuristic { ... };
```

## Existing surfaces touched

<!-- Ideally: none. If yes, list each file and why. -->

## Plan

- [ ] Header(s) in `pae/include/pae/<area>/`
- [ ] Implementation in `pae/src/<area>/`
- [ ] Registration in `pae/src/factory/register_default.cpp`
- [ ] Tests in `pae/tests/test_<area>.cpp`
- [ ] Docs update (`docs/<RELEVANT>.md`)
- [ ] CHANGELOG entry (`[Unreleased]`)
- [ ] FEATURES.md row added or updated

## Target version

<!-- See ROADMAP.md. e.g. v1.1, v1.2, v2.0. -->

## Risks / open questions

<!-- Algorithmic ambiguity, perf cost, dep introduction, etc. -->
