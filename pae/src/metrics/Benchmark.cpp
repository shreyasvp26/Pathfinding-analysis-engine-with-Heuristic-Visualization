#include "pae/metrics/Benchmark.hpp"

#include "pae/algorithms/AStar.hpp"
#include "pae/algorithms/IPathfinder.hpp"
#include "pae/algorithms/RunConfig.hpp"
#include "pae/core/Grid.hpp"
#include "pae/factory/Registry.hpp"
#include "pae/heuristics/IHeuristic.hpp"
#include "pae/visualization/NullVisualizer.hpp"

namespace pae::metrics {

namespace {

bool needsHeuristic(const std::string& algo) {
    return algo == "astar";
}

}  // namespace

std::vector<BenchmarkRun> Benchmark::sweep(
    const core::Grid&                  grid,
    core::Span<const std::string>      algoNames,
    core::Span<const std::string>      heuristicNames,
    Config                             cfg) {
    std::vector<BenchmarkRun> runs;
    runs.reserve(static_cast<std::size_t>(algoNames.size()) *
                 static_cast<std::size_t>(heuristicNames.size() + 1) *
                 static_cast<std::size_t>(cfg.repetitions));

    viz::NullVisualizer nullViz;
    auto* vizPtr = cfg.useNullViz ? &nullViz : nullptr;

    for (const auto& algoName : algoNames) {
        const bool wantsH = needsHeuristic(algoName);
        const auto& heurs = wantsH ? heuristicNames : core::Span<const std::string>{};

        // For non-heuristic algorithms we still want one row.
        const std::size_t hCount = wantsH ? heurs.size() : 1;
        for (std::size_t hi = 0; hi < hCount; ++hi) {
            const std::string heurName = wantsH ? heurs[hi] : std::string{};

            std::unique_ptr<heur::IHeuristic> heuristic;
            std::unique_ptr<algo::IPathfinder> pathfinder;
            if (wantsH) {
                heuristic = factory::Registry<heur::IHeuristic>::instance().create(heurName);
            }
            if (algoName == "astar" && heuristic) {
                pathfinder = std::make_unique<algo::AStar>(*heuristic);
            } else {
                pathfinder = factory::Registry<algo::IPathfinder>::instance().create(algoName);
            }

            algo::RunConfig rc{};
            Metrics scratch{};

            // Warmup
            for (int w = 0; w < cfg.warmup; ++w) {
                scratch.reset();
                (void)pathfinder->run(grid, rc, vizPtr, &scratch);
            }
            // Measured
            for (int r = 0; r < cfg.repetitions; ++r) {
                Metrics m{};
                (void)pathfinder->run(grid, rc, vizPtr, &m);
                runs.push_back(BenchmarkRun{algoName, heurName, m, r});
            }
        }
    }
    return runs;
}

}  // namespace pae::metrics
