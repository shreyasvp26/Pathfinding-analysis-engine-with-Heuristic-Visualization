# Pathfinding Analysis Engine with Heuristic Visualization (PAE)

> A modular, OOP-first C++17 engine that compares pathfinding algorithms
> (A\*, Dijkstra, BFS) under pluggable heuristics (Manhattan, Euclidean,
> Chebyshev) on 2D grids — with step-by-step CLI visualization and
> per-run performance metrics.

This repository is a **production-quality engineering blueprint**, not a
toy visualizer. It is designed to demonstrate:

- Strong problem-solving and algorithmic depth (A\*, Dijkstra, BFS, with
  optimality / admissibility proofs in `docs/ALGORITHMS.md`).
- Clean Low-Level Design (LLD) with abstraction, inheritance,
  polymorphism, and separation of concerns.
- Extensibility — add a new algorithm or heuristic without touching the
  CLI, the metrics layer, or any other algorithm.
- Measurable performance comparison across algorithms and heuristics.
- A real Software Development Life Cycle (SDLC) modelled on the
  `ssm_calender` and `Jyotish AI` projects in this repo collection
  (Copilot agents, prompt library, CI/CD, structured docs).

---

## TL;DR — Quick start

```bash
cd pae
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Run a single algorithm with step-by-step CLI visualization
./build/pae --map maps/maze_20x20.txt \
            --algo astar \
            --heuristic manhattan \
            --visualize step

# Compare all algorithms on the same map (benchmark mode)
./build/pae --map maps/maze_50x50.txt --benchmark

# Run unit tests
ctest --test-dir build --output-on-failure
```

---

## Where to look

| Area | Path |
|------|------|
| **Project plan / blueprint** | [`docs/IMPLEMENTATION_PLAN.md`](docs/IMPLEMENTATION_PLAN.md) |
| **System architecture** | [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) |
| **Low-Level Design (classes, UML)** | [`docs/LLD.md`](docs/LLD.md) |
| **Algorithm deep dive** | [`docs/ALGORITHMS.md`](docs/ALGORITHMS.md) |
| **Heuristic design** | [`docs/HEURISTICS.md`](docs/HEURISTICS.md) |
| **Data-structure choices** | [`docs/DATA_STRUCTURES.md`](docs/DATA_STRUCTURES.md) |
| **Folder structure** | [`docs/FOLDER_STRUCTURE.md`](docs/FOLDER_STRUCTURE.md) |
| **Requirements** | [`docs/REQUIREMENTS.md`](docs/REQUIREMENTS.md) |
| **Feature tracker** | [`docs/FEATURES.md`](docs/FEATURES.md) |
| **Bug tracker** | [`docs/BUGS.md`](docs/BUGS.md) |
| **Changelog** | [`docs/CHANGELOG.md`](docs/CHANGELOG.md) |
| **Roadmap** | [`docs/ROADMAP.md`](docs/ROADMAP.md) |
| **Testing strategy** | [`docs/TESTING.md`](docs/TESTING.md) |
| **Performance plan** | [`docs/PERFORMANCE.md`](docs/PERFORMANCE.md) |
| **Extensions** | [`docs/EXTENSIONS.md`](docs/EXTENSIONS.md) |
| **Agent definitions** | [`AGENTS.md`](AGENTS.md) |
| **Copilot master rules** | [`.github/copilot-instructions.md`](.github/copilot-instructions.md) |
| **GitHub setup** | [`.github/SETUP_GITHUB.md`](.github/SETUP_GITHUB.md) |

---

## Tech stack

| Layer | Choice | Why |
|-------|--------|-----|
| Language | **C++17** | RAII, `std::optional`, `if constexpr`, structured bindings; broadly supported. |
| Build | **CMake ≥ 3.20** | Multi-platform, well-understood, integrates with CTest. |
| Tests | **Catch2 v3** (FetchContent) | Header-only-ish, expressive `REQUIRE` semantics, no external deps. |
| Bench | **Custom microbench harness** + optional **Google Benchmark** | Avoid framework lock-in for the core; opt-in to GB for stable numbers. |
| Lint | **clang-tidy + clang-format** (LLVM style) | Industry-standard, deterministic CI. |
| CI | **GitHub Actions** | Parity with `ssm_calender` / `Jyotish AI`. |
| Docs | **Markdown** | Render natively on GitHub; same pattern as sibling projects. |

External dependencies are **fetched at configure time** (Catch2 only) — there
is no system-wide install requirement beyond a C++17 compiler and CMake.

---

## What this is **not**

- Not a game engine, not a graphics-API demo, not a GUI app.
- Not a "framework" — it is one binary plus reusable libraries.
- Not coupled to any concrete heuristic, algorithm, map format, or
  visualization channel — every one of those is behind an interface.

---

## Sibling projects (SDLC parents)

This project follows the same SDLC, agent, prompt, and CI structure as:

- [`../ssm_calender`](../ssm_calender) — Marathi panchang calendar
  (TypeScript / Next.js) — the **structural parent**: `docs/`, `.github/`,
  `AGENTS.md`, `prompts/`, `instructions/`, `workflows/`.
- [`../Jyotish AI`](../Jyotish%20AI) — Vedic astrology engine
  (Python / FastAPI / Next.js) — the **methodological parent**:
  phased master plan, layer-by-layer ship prompts, CHANGELOG-driven
  releases.

If you have worked on either of those repos, the workflow here is
identical — only the tools (`pytest` → `ctest`, `tsc` → `clang-tidy`,
`vitest` → `Catch2`) change.

---

## License

MIT — see [`LICENSE`](LICENSE).
