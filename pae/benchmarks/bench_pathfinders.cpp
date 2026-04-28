#include <iostream>
#include <string>
#include <vector>

#include "pae/factory/Registry.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/metrics/Benchmark.hpp"
#include "pae/metrics/Report.hpp"

int main(int argc, char** argv) {
    std::string mapPath  = "pae/maps/maze_20x20.txt";
    std::string format   = "table";
    int         reps     = 30;
    int         warmup   = 3;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--map" && i + 1 < argc) {
            mapPath = argv[++i];
        } else if (a == "--format" && i + 1 < argc) {
            format = argv[++i];
        } else if (a == "--reps" && i + 1 < argc) {
            reps = std::stoi(argv[++i]);
        } else if (a == "--warmup" && i + 1 < argc) {
            warmup = std::stoi(argv[++i]);
        } else if (a == "--json") {
            format = "json";
        } else if (a == "--csv") {
            format = "csv";
        }
    }

    pae::factory::registerAll();

    auto grid = pae::io::GridLoader::loadFromFile(mapPath);

    const std::vector<std::string> algos {"astar", "dijkstra", "bfs"};
    const std::vector<std::string> heurs {"manhattan", "euclidean", "chebyshev"};

    pae::metrics::Benchmark::Config cfg{};
    cfg.repetitions = reps;
    cfg.warmup      = warmup;

    auto runs = pae::metrics::Benchmark::sweep(
        grid,
        pae::core::Span<const std::string>{algos.data(), algos.size()},
        pae::core::Span<const std::string>{heurs.data(), heurs.size()},
        cfg);

    pae::metrics::Report report{std::move(runs)};
    if (format == "json") {
        report.writeJson(std::cout);
    } else if (format == "csv") {
        report.writeCsv(std::cout);
    } else {
        report.printTable(std::cout);
    }
    return 0;
}
