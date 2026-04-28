---
mode: 'agent'
description: 'Generate Catch2 tests for an existing module.'
---

Generate tests for:

${input:module:Module to test — e.g. "pae::algo::AStar" or "pae/src/heuristics/Manhattan.cpp"}

## Steps

1. Read `docs/TESTING.md` (the master) and the module's source.
2. Determine the test labels: `[grid]`, `[algo]`, `[heuristic]`,
   `[viz]`, `[cross]`, `[property]`, `[perf]`.
3. Determine which patterns from `docs/TESTING.md` §3, §4, §5 apply:
   - For algorithms: EC-01 → EC-08 + cross-algo.
   - For heuristics: property battery + sampled admissibility.
   - For Grid / GridLoader: bounds, neighbours, malformed inputs.
   - For visualizer: snapshot + reset + null case.
4. Create or extend `pae/tests/test_<area>.cpp` with `TEST_CASE`s
   following the templates in
   `.github/instructions/testing.instructions.md`.
5. Update `pae/tests/CMakeLists.txt` if a new file was added.
6. Run: `cmake --build pae/build -j && ctest --test-dir pae/build -L <label> --output-on-failure`.
7. Tests must be **deterministic** and **fast** (< 50 ms each on a
   dev box; < 200 ms on Debug+ASan).

## Hard rules

- No `std::cout` (use Catch2 `CAPTURE`/`INFO`).
- No `sleep_for`.
- No `random_device`. Use `std::mt19937{seed}` if randomness is needed.
- No globals.
- One behaviour per `TEST_CASE`. If the case description has "and",
  it's two cases.

## Done when

- `ctest` is green for the new tests AND the existing ones.
- ASan + UBSan green on Debug.
- The PR description lists which `REQUIREMENTS.md` rows now have a
  matching test.
