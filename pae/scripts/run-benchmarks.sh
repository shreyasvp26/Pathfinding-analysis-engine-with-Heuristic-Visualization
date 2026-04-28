#!/usr/bin/env bash
# Run pae_bench over every map in pae/maps/ and write JSON output.

set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

if [[ ! -x pae/build-rel/benchmarks/pae_bench ]]; then
    echo "pae_bench not built. Building Release first..."
    cmake -S . -B pae/build-rel -DCMAKE_BUILD_TYPE=Release
    cmake --build pae/build-rel -j
fi

mkdir -p pae/benchmarks/results
ts=$(date -u +"%Y%m%dT%H%M%SZ")

for m in pae/maps/*.txt; do
    out="pae/benchmarks/results/$(basename "${m%.txt}")_${ts}.json"
    echo "==> $m -> $out"
    ./pae/build-rel/benchmarks/pae_bench --map "$m" --json > "$out"
done

echo "Done. Results in pae/benchmarks/results/"
