#!/usr/bin/env bash
# Run clang-format in-place across the entire pae/ tree.
# Use the project's .clang-format profile (LLVM-derived).

set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

mapfile -t FILES < <(git ls-files \
    'pae/include/**/*.hpp' \
    'pae/src/**/*.cpp' \
    'pae/tests/**/*.cpp' \
    'pae/tests/**/*.hpp' \
    'pae/benchmarks/**/*.cpp')

if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "No C++ files found."
    exit 0
fi

clang-format -i --style=file "${FILES[@]}"
echo "Formatted ${#FILES[@]} files."
