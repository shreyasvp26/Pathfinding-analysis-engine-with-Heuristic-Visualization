// Perf-budget tests — F-404.
//
// These run only in builds configured with `-DPAE_PERF_BUDGET=ON` (use
// `--preset perf`). They are NOT meant to be tight performance
// assertions; their job is to catch order-of-magnitude regressions
// like "someone accidentally made A* O(N^2)" or "someone deleted the
// closed-set check". Budgets are intentionally generous (≈ 50–200×
// the current Linux Release median on a shared GitHub runner) so the
// suite stays flake-free while still failing on real degradations.
//
// Each TEST_CASE is tagged `[perf]` so `catch_discover_tests` lifts
// that into a CMake test label; the `perf-budget` job in `ci.yml`
// runs `ctest -L perf` and only invokes these.
//
// To tighten budgets later: drop the absolute thresholds and replace
// them with deltas vs `pae/benchmarks/baselines/<map>.json` (F-405).

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "pae/core/Span.hpp"
#include "pae/factory/Registry.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/metrics/Benchmark.hpp"

using pae::factory::registerAll;
using pae::io::GridLoader;
using pae::metrics::Benchmark;
using pae::metrics::BenchmarkRun;

namespace {

constexpr int kReps   = 5;
constexpr int kWarmup = 1;

// Returns the median wall-time (us) for the run-set matching (algo, heur).
// Empty heur matches Dijkstra/BFS rows in the sweep output.
std::int64_t medianWallMicros(const std::vector<BenchmarkRun>& runs,
                              const std::string&               algo,
                              const std::string&               heur) {
    std::vector<std::int64_t> wallTimes;
    for (const auto& r : runs) {
        if (r.algoName == algo && r.heuristicName == heur) {
            wallTimes.push_back(r.metrics.wallMicros);
        }
    }
    REQUIRE_FALSE(wallTimes.empty());
    std::sort(wallTimes.begin(), wallTimes.end());
    return wallTimes[wallTimes.size() / 2];
}

}  // namespace

TEST_CASE("perf budget: maze_20x20 (every config under 50 ms median)",
          "[perf][algo][heuristic]") {
    registerAll();
    auto grid = GridLoader::loadFromFile("../maps/maze_20x20.txt");

    const std::vector<std::string> algos {"astar", "dijkstra", "bfs"};
    const std::vector<std::string> heurs {"manhattan", "euclidean", "chebyshev", "octile"};
    Benchmark::Config cfg{kReps, kWarmup, true};
    const auto runs = Benchmark::sweep(
        grid,
        pae::core::Span<const std::string>{algos.data(), algos.size()},
        pae::core::Span<const std::string>{heurs.data(), heurs.size()},
        cfg);

    constexpr std::int64_t kBudgetMicros = 50'000;  // 50 ms

    for (const auto& a : algos) {
        if (a == "astar") {
            for (const auto& h : heurs) {
                const auto med = medianWallMicros(runs, a, h);
                INFO("(" << a << ", " << h << ") median = " << med << " us");
                REQUIRE(med <= kBudgetMicros);
            }
        } else {
            const auto med = medianWallMicros(runs, a, "");
            INFO("(" << a << ", -) median = " << med << " us");
            REQUIRE(med <= kBudgetMicros);
        }
    }
}

TEST_CASE("perf budget: open_arena_50x50 (every config under 500 ms median)",
          "[perf][algo][heuristic]") {
    registerAll();
    auto grid = GridLoader::loadFromFile("../maps/open_arena_50x50.txt");

    const std::vector<std::string> algos {"astar", "dijkstra", "bfs"};
    const std::vector<std::string> heurs {"manhattan", "euclidean", "chebyshev", "octile"};
    Benchmark::Config cfg{kReps, kWarmup, true};
    const auto runs = Benchmark::sweep(
        grid,
        pae::core::Span<const std::string>{algos.data(), algos.size()},
        pae::core::Span<const std::string>{heurs.data(), heurs.size()},
        cfg);

    // 500 ms budget: ~1000× the local Apple Debug median (~505 us).
    // Sanitiser builds and slow Windows runners would still pass.
    constexpr std::int64_t kBudgetMicros = 500'000;

    for (const auto& a : algos) {
        if (a == "astar") {
            for (const auto& h : heurs) {
                const auto med = medianWallMicros(runs, a, h);
                INFO("(" << a << ", " << h << ") median = " << med << " us");
                REQUIRE(med <= kBudgetMicros);
            }
        } else {
            const auto med = medianWallMicros(runs, a, "");
            INFO("(" << a << ", -) median = " << med << " us");
            REQUIRE(med <= kBudgetMicros);
        }
    }
}

TEST_CASE("perf budget: optimality is preserved (every config agrees on path cost)",
          "[perf][algo][heuristic][correctness]") {
    registerAll();
    auto grid = GridLoader::loadFromFile("../maps/maze_50x50.txt");

    const std::vector<std::string> algos {"astar", "dijkstra", "bfs"};
    const std::vector<std::string> heurs {"manhattan", "euclidean", "chebyshev", "octile"};
    Benchmark::Config cfg{1, 0, true};   // single rep, no warmup; we want correctness only
    const auto runs = Benchmark::sweep(
        grid,
        pae::core::Span<const std::string>{algos.data(), algos.size()},
        pae::core::Span<const std::string>{heurs.data(), heurs.size()},
        cfg);

    REQUIRE_FALSE(runs.empty());
    const auto baselineCost = runs.front().metrics.pathCost;
    REQUIRE(baselineCost > 0);
    for (const auto& r : runs) {
        INFO("(" << r.algoName << ", " << r.heuristicName << ") cost = " << r.metrics.pathCost);
        REQUIRE(r.metrics.pathCost == baselineCost);
    }
}
