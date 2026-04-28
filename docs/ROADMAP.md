# Roadmap — Pathfinding Analysis Engine

A long-horizon view that complements
[`IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md) (which is
phase-detailed for v0.1 → v1.0). This file looks past v1.0.

Mirrors the structure of `Jyotish AI/docs/ROADMAP.md` and
`ssm_calender/docs/CHANGELOG.md`'s long view.

---

## 1. Horizon at a glance

```
v0.1 ──► v0.2 ──► v0.3 ──► v0.4 ──► v1.0 ──► v1.1 ──► v1.2 ──► v1.3 ──► v2.0
   MVP    Trio     Hard    Bench    Polish   Octile   Bidi     JPS     3D / API
   BFS    + A*     -ening   gating          + maze    A*      / Theta*  break
          + Dij    + diag                   gen
```

---

## 2. v0.x cycle (single-binary, single-thread)

| Tag | Theme | Highlights |
|-----|-------|------------|
| `v0.1.0` | MVP vertical slice | Grid, GridLoader, BFS, Manhattan, Final-only viz |
| `v0.2.0` | Algorithm trio | Add Dijkstra, A\*, Euclidean, step viz, basic benchmark |
| `v0.3.0` | Robustness | Chebyshev, 8-conn, ASan/UBSan in CI, edge-case maps |
| `v0.4.0` | Benchmark hardening | Sweep, median/p95, baselines committed, perf gating |
| `v1.0.0` | Polish + release | Full --help, CHANGELOG entry, three-OS binaries on Releases |

---

## 3. v1.x — additive, never breaking

| Tag | Content | Owner | Risk |
|-----|---------|-------|------|
| `v1.1.0` | Octile heuristic; random maze generator (`--gen maze`) | `@heuristic`, `@core` | low |
| `v1.2.0` | Bidirectional A\* | `@algorithm` | medium (termination rules) |
| `v1.3.0` | Jump Point Search (JPS) on unweighted grids | `@algorithm` | medium |
| `v1.4.0` | Theta\* (any-angle) | `@algorithm` | medium-high |
| `v1.5.0` | True RSS metric on POSIX/Windows; `--profile cpu` instrumented hooks | `@perf` | low |
| `v1.6.0` | Optional GUI visualizer (Raylib) — only built when `-DPAE_GUI=ON` | `@viz` | medium (first GUI dep) |
| `v1.7.0` | Logging framework, JSON map format, additional sample maps | `@core` | low |

Each v1.x is **additive**. No public header in `pae/include/pae/` may
have a removed or signature-changed method between v1.0 and v1.x; new
methods OK.

---

## 4. v2.0 — possibly-breaking

| Candidate | Why breaking | Effort |
|-----------|-------------|--------|
| 3D grids | `Coord` becomes a 3-tuple; `Grid` becomes 3D-indexed; everything downstream changes signature. | very high |
| Hex grids | Different neighbour topology; same `IPathfinder` contract though. Could be additive (`HexGrid` parallel type). | medium |
| Engine-as-library API for hosts | Public C ABI for Python/Go bindings; would dwarf the CLI in surface area. | very high |
| Replanning (D\*, LPA\*) | `Grid` immutability assumption is broken. | very high |

We do **not** commit to v2.0 yet. We re-evaluate when v1.x stops
producing useful additions.

---

## 5. Cross-cutting backlog (no version assigned)

| Theme | Note |
|-------|------|
| Cross-platform CI matrix expansion | Add `gcc-13`, `clang-18`, MSVC `2022/17.8`. Currently we pin `gcc-11`+`clang-14`+`MSVC-19.30`. |
| Code coverage badge in README | Use Codecov or self-hosted. |
| Docs site (mkdocs / docusaurus) | If `docs/` exceeds ~10k lines (currently ~5k); not pressing. |
| Benchmark dashboard | Aggregate JSON reports across releases; dashboard hosted on GitHub Pages from `gh-pages` branch. |
| Fuzz testing the loader | `cargo-fuzz`-style with libFuzzer; `GridLoader` is the only obvious fuzz target. |

---

## 6. Decisions log

Major decisions that shape the roadmap. Each is a one-sentence
**why**, with a link to the issue/PR that discussed it.

| ID | Decision | Date | Reason |
|----|---------|------|--------|
| D-001 | C++17, not C++20 | 2026-04-29 | Broad compiler support; we don't need concepts/coroutines for v1. |
| D-002 | CMake + FetchContent, not vcpkg/Conan | 2026-04-29 | One-binary student project; no global package manager. |
| D-003 | Catch2 v3, not GoogleTest | 2026-04-29 | Smaller, simpler, no protobuf transitive. |
| D-004 | `priority_queue<Node, vector<Node>>` for the open set, not Fibonacci heap | 2026-04-29 | Constants beat asymptotics at our sizes. |
| D-005 | Implicit closed set via `gScore` | 2026-04-29 | Saves a container; consistent heuristics make it correct. |
| D-006 | One concrete class per algorithm, not a templated `BestFirst<Cmp>` | 2026-04-29 | Honest naming + OOP demonstrability. |
| D-007 | Visualizer is event-based, never called from inside `priority_queue` | 2026-04-29 | Keeps algorithms pure; testable; perf-controllable. |
| D-008 | Three-OS first-class CI matrix | 2026-04-29 | Mirrors SSM/Jyotish; catches MSVC oddities early. |

This log is append-only. Reverting a decision means appending a new
row that supersedes the old one (and updating the relevant doc).

---

## 7. Communication of roadmap changes

When a planned tag's contents change:

1. Update [`FEATURES.md`](FEATURES.md) row state.
2. Update this file's table for the affected tag.
3. Open an issue with the `roadmap` label so the discussion is
   indexed and discoverable.

We do not "soft-deprecate" features in roadmap conversation. Either
they're shipped, planned, or removed — never "maybe."
