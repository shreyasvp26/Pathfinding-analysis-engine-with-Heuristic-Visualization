#!/usr/bin/env bash
# Run pae_bench over every map in pae/maps/ and write JSON output.

set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

BENCH=pae/build-rel/benchmarks/pae_bench

if [[ ! -x "$BENCH" ]]; then
    echo "pae_bench not built. Building Release first..."
    cmake -S pae -B pae/build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DPAE_BUILD_TESTS=OFF
    cmake --build pae/build-rel -j
fi

mkdir -p pae/benchmarks/results
ts=$(date -u +"%Y%m%dT%H%M%SZ")

for m in pae/maps/*.txt; do
    out="pae/benchmarks/results/$(basename "${m%.txt}")_${ts}.json"
    echo "==> $m -> $out"
    "$BENCH" --map "$m" --json > "$out"
done

echo "Done. Results in pae/benchmarks/results/"
