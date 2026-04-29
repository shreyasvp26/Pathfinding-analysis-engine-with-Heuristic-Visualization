#include "pae/cli/App.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "pae/algorithms/AStar.hpp"
#include "pae/algorithms/BFS.hpp"
#include "pae/algorithms/Dijkstra.hpp"
#include "pae/algorithms/IPathfinder.hpp"
#include "pae/factory/Registry.hpp"
#include "pae/heuristics/IHeuristic.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/metrics/Benchmark.hpp"
#include "pae/metrics/Metrics.hpp"
#include "pae/metrics/Report.hpp"
#include "pae/metrics/Rss.hpp"
#include "pae/visualization/CliVisualizer.hpp"
#include "pae/visualization/NullVisualizer.hpp"

namespace pae::cli {

int App::run() {
    if (cfg_.showHelp) {
        std::cout << helpText();
        return 0;
    }
    if (cfg_.showVersion) {
        std::cout << versionText() << "\n";
        return 0;
    }

    auto grid = io::GridLoader::loadFromFile(cfg_.mapPath);

    if (cfg_.algo == "astar" && cfg_.diagonal && cfg_.heuristic == "manhattan") {
        std::cerr
            << "pae: refusing astar+manhattan with --diagonal "
               "(Manhattan is inadmissible for 8-conn). Use chebyshev or euclidean.\n";
        return 1;
    }

    if (cfg_.benchmark) {
        const std::vector<std::string> algoNames     {"astar", "dijkstra", "bfs"};
        const std::vector<std::string> heuristicNames{"manhattan", "euclidean", "chebyshev", "octile"};
        metrics::Benchmark::Config bcfg{};
        auto runs = metrics::Benchmark::sweep(
            grid,
            core::Span<const std::string>{algoNames.data(),     algoNames.size()},
            core::Span<const std::string>{heuristicNames.data(), heuristicNames.size()},
            bcfg);
        metrics::Report report{std::move(runs)};
        report.printTable(std::cout);
        report.printCharts(std::cout, cfg_.color);
        return 0;
    }

    // Single-run mode.
    std::unique_ptr<heur::IHeuristic>  heuristic;
    std::unique_ptr<algo::IPathfinder> algorithm;

    if (cfg_.algo == "astar") {
        heuristic = factory::Registry<heur::IHeuristic>::instance().create(cfg_.heuristic);
        algorithm = std::make_unique<algo::AStar>(*heuristic);
    } else {
        algorithm = factory::Registry<algo::IPathfinder>::instance().create(cfg_.algo);
    }

    viz::CliConfig cliCfg{};
    cliCfg.mode  = cfg_.vizMode;
    cliCfg.color = cfg_.color;
    cliCfg.clearTerm = cfg_.vizMode == viz::VizMode::Step;

    viz::CliVisualizer  cliViz{cliCfg, std::cout};
    viz::NullVisualizer nullViz;
    viz::IVisualizer*   viz = (cfg_.vizMode == viz::VizMode::None)
                              ? static_cast<viz::IVisualizer*>(&nullViz)
                              : static_cast<viz::IVisualizer*>(&cliViz);

    algo::RunConfig rc{};
    rc.diagonal = cfg_.diagonal;
    rc.seed     = cfg_.seed;

    metrics::Metrics m{};
    const auto       result = algorithm->run(grid, rc, viz, &m);

    std::cout
        << "\n--- summary ---\n"
        << "found:           " << (result.found ? "yes" : "no") << '\n'
        << "path_length:     " << m.pathLength << '\n'
        << "path_cost:       " << m.pathCost   << '\n'
        << "nodes_expanded:  " << m.nodesExpanded << '\n'
        << "nodes_enqueued:  " << m.nodesEnqueued << '\n'
        << "wall_us:         " << m.wallMicros    << '\n'
        << "approx_peak_B:   " << m.approxPeakBytes << '\n';
    if constexpr (metrics::trueRssAvailable()) {
        std::cout << "rss_delta_B:     " << m.rssDeltaBytes << '\n';
    }
    return result.found ? 0 : 1;
}

}  // namespace pae::cli
