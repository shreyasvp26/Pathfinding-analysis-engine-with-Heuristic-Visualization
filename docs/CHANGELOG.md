# Changelog — Pathfinding Analysis Engine

All notable changes to this project are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Conventions (parity with `ssm_calender` and `Jyotish AI`):
- `[Unreleased]` is the holding pen; entries collect there during the
  cycle, then the heading is renamed to the new tag at release time.
- Entry shape: `- type(scope): one-sentence change ([#PR](url))`.
- Types: `feat`, `fix`, `docs`, `refactor`, `perf`, `test`, `build`,
  `ci`, `chore`.

---

## [Unreleased]

### Added
- docs: comprehensive engineering blueprint — `ARCHITECTURE.md`, `LLD.md`, `ALGORITHMS.md`, `HEURISTICS.md`, `DATA_STRUCTURES.md`, `TESTING.md`, `PERFORMANCE.md`, `EXTENSIONS.md`, `ROADMAP.md`, `IMPLEMENTATION_PLAN.md`, `FOLDER_STRUCTURE.md`, `REQUIREMENTS.md`, `FEATURES.md`, `BUGS.md`.
- docs: `AGENTS.md` orchestration map (Core, Algorithm, Heuristic, Visualization, Performance, QA, Build).
- chore: repo skeleton — `README.md`, `SECURITY.md`, `LICENSE` (MIT), `.gitignore`, `.gitattributes`, `.editorconfig`, `.clang-format`, `.clang-tidy`, `.pre-commit-config.yaml`.
- build: top-level `CMakeLists.txt` delegating to `pae/CMakeLists.txt`.
- build: `.github/` scaffolding — `copilot-instructions.md`, `SETUP_GITHUB.md`, agent personas, instruction files, prompt library, CI workflows.

### Changed
- _none_

### Deprecated
- _none_

### Removed
- _none_

### Fixed
- _none_

### Security
- security: documented threat model in `SECURITY.md`; pinned Catch2 dependency in `FetchContent`.

---

## How releases work

When a tag is cut:

1. The maintainer runs `pae/scripts/bump-version.sh <new>` which
   updates the version line in `pae/CMakeLists.txt` and renames the
   `[Unreleased]` heading in this file to `[<new>] - YYYY-MM-DD`.
2. A fresh `[Unreleased]` block is added at the top.
3. Commit message: `chore(release): v<new>`.
4. `git tag v<new>` and `git push --tags`.
5. The `release.yml` workflow attaches Linux/macOS/Windows binaries to
   the GitHub Release.

Planned releases are listed in [`ROADMAP.md`](ROADMAP.md). Per-version
content commitments are in [`IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md) §6.

---

## Past releases

_(none yet — first tag will be `v0.1.0`)_
