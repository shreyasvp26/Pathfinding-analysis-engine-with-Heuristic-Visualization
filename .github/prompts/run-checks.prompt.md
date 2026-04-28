---
mode: 'agent'
description: 'Run the full verification suite sequentially. Slowest but most reliable.'
---

Run all verification checks **sequentially** and report results.

## Sequence

1. Configure Debug + sanitizers:
   ```bash
   cmake -S . -B pae/build -DCMAKE_BUILD_TYPE=Debug -DPAE_SANITIZERS=ON
   ```
2. Configure Release:
   ```bash
   cmake -S . -B pae/build-rel -DCMAKE_BUILD_TYPE=Release
   ```
3. Build Debug:
   ```bash
   cmake --build pae/build -j
   ```
4. Build Release:
   ```bash
   cmake --build pae/build-rel -j
   ```
5. ctest Debug (sanitizer-instrumented):
   ```bash
   ctest --test-dir pae/build --output-on-failure
   ```
6. ctest Release:
   ```bash
   ctest --test-dir pae/build-rel --output-on-failure
   ```
7. clang-format dry-run:
   ```bash
   clang-format --dry-run --Werror $(git ls-files 'pae/**/*.hpp' 'pae/**/*.cpp')
   ```
8. clang-tidy:
   ```bash
   clang-tidy -p pae/build $(git ls-files 'pae/src/**/*.cpp')
   ```
9. Cross-algorithm equivalence:
   ```bash
   ctest --test-dir pae/build -R cross_algorithm --output-on-failure
   ```

## Report

Present in a table (steps × status × details). Stop on the first
failing step and surface the relevant excerpt; do not auto-retry.

For a faster but parallel version, use `/parallel-checks`.
