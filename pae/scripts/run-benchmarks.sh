#!/usr/bin/env bash
# Run pae_bench over every map in pae/maps/ and write JSON output.
# When the run finishes, also render a self-contained HTML dashboard
# that turns the JSON results into Chart.js bar charts.
# Skip the dashboard step with --no-dashboard.

set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

build_dashboard=1
for arg in "$@"; do
    case "$arg" in
        --no-dashboard) build_dashboard=0 ;;
        *) echo "run-benchmarks: unknown arg: $arg" >&2; exit 2 ;;
    esac
done

BENCH=pae/build-rel/benchmarks/pae_bench

if [[ ! -x "$BENCH" ]]; then
    echo "pae_bench not built. Building Release first..."
    cmake -S pae -B pae/build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DPAE_BUILD_TESTS=OFF
    cmake --build pae/build-rel -j
fi

mkdir -p pae/benchmarks/results
ts=$(date -u +"%Y%m%dT%H%M%SZ")

# Use a per-run subdirectory so the dashboard only includes the JSON
# files from this exact sweep and not stale historical ones.
results_dir="pae/benchmarks/results/${ts}"
mkdir -p "$results_dir"

for m in pae/maps/*.txt; do
    out="${results_dir}/$(basename "${m%.txt}").json"
    echo "==> $m -> $out"
    "$BENCH" --map "$m" --json > "$out"
done

echo "Raw JSON results in ${results_dir}/"

if (( build_dashboard )); then
    dashboard_path="${results_dir}/dashboard.html"
    if python3 pae/scripts/render_dashboard.py "${results_dir}" \
        -o "${dashboard_path}"; then
        latest="pae/benchmarks/results/dashboard-latest.html"
        ln -sf "${ts}/dashboard.html" "${latest}"
        echo "Dashboard at ${dashboard_path}"
        echo "(also linked from ${latest})"
    else
        echo "warning: dashboard render failed; raw JSON is still in ${results_dir}/" >&2
    fi
fi

echo "Done."
