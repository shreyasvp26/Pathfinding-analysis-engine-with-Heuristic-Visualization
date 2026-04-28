#!/usr/bin/env bash
# Sequential verification suite: configure → build → ctest → format/tidy.
# Mirrors .github/prompts/run-checks.prompt.md.

set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

echo "==> configure (Debug + sanitizers)"
cmake -S . -B pae/build -DCMAKE_BUILD_TYPE=Debug -DPAE_SANITIZERS=ON

echo "==> configure (Release)"
cmake -S . -B pae/build-rel -DCMAKE_BUILD_TYPE=Release

echo "==> build (Debug)"
cmake --build pae/build -j

echo "==> build (Release)"
cmake --build pae/build-rel -j

echo "==> ctest (Debug + sanitizers)"
ctest --test-dir pae/build --output-on-failure

echo "==> ctest (Release)"
ctest --test-dir pae/build-rel --output-on-failure

echo "==> clang-format dry-run"
mapfile -t FILES < <(git ls-files \
    'pae/include/**/*.hpp' \
    'pae/src/**/*.cpp' \
    'pae/tests/**/*.cpp' \
    'pae/tests/**/*.hpp' \
    'pae/benchmarks/**/*.cpp')
if [[ ${#FILES[@]} -gt 0 ]]; then
    clang-format --dry-run --Werror "${FILES[@]}"
fi

echo "==> clang-tidy"
mapfile -t SRC < <(git ls-files 'pae/src/**/*.cpp')
if [[ ${#SRC[@]} -gt 0 ]]; then
    clang-tidy -p pae/build "${SRC[@]}"
fi

echo "==> all checks passed"
