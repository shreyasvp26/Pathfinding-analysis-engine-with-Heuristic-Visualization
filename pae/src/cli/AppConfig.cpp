#include "pae/cli/AppConfig.hpp"

#include <cstring>
#include <sstream>
#include <string>

#include "pae/io/Errors.hpp"

namespace pae::cli {

namespace {

[[nodiscard]] viz::VizMode parseVizMode(std::string_view s) {
    if (s == "none")  return viz::VizMode::None;
    if (s == "final") return viz::VizMode::Final;
    if (s == "step")  return viz::VizMode::Step;
    throw UsageError{"unknown --visualize value: " + std::string{s}};
}

}  // namespace

std::string helpText() {
    std::ostringstream os;
    os << "pae — Pathfinding Analysis Engine\n\n"
       << "Usage: pae --map <path> [options]\n\n"
       << "Required:\n"
       << "  --map <path>          Map file (ASCII grid)\n\n"
       << "Algorithm:\n"
       << "  --algo <name>         astar | dijkstra | bfs                       (default: astar)\n"
       << "  --heuristic <name>    manhattan | euclidean | chebyshev | octile   (default: manhattan)\n"
       << "  --diagonal            Allow 8-conn movement\n\n"
       << "Visualization:\n"
       << "  --visualize <mode>    none | final | step           (default: step)\n"
       << "  --no-color            Disable ANSI colour\n\n"
       << "Modes:\n"
       << "  --benchmark           Run all algorithms on the map; print comparison table\n"
       << "  --seed <int>          Seed auxiliary tie-breaking\n\n"
       << "  --help                Show this help and exit\n"
       << "  --version             Print version and exit\n";
    return os.str();
}

std::string versionText() {
    return "pae 0.1.0";
}

AppConfig parseArgs(int argc, char** argv) {
    AppConfig cfg{};
    auto need = [&](int i) {
        if (i + 1 >= argc) {
            throw UsageError{"missing value after " + std::string{argv[i]}};
        }
    };
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--help") {
            cfg.showHelp = true;
        } else if (a == "--version") {
            cfg.showVersion = true;
        } else if (a == "--map") {
            need(i);
            cfg.mapPath = argv[++i];
        } else if (a == "--algo") {
            need(i);
            cfg.algo = argv[++i];
        } else if (a == "--heuristic") {
            need(i);
            cfg.heuristic = argv[++i];
        } else if (a == "--visualize" || a == "--visualise") {
            need(i);
            cfg.vizMode = parseVizMode(argv[++i]);
        } else if (a == "--diagonal") {
            cfg.diagonal = true;
        } else if (a == "--no-color" || a == "--no-colour") {
            cfg.color = false;
        } else if (a == "--benchmark") {
            cfg.benchmark = true;
        } else if (a == "--seed") {
            need(i);
            cfg.seed = std::stoull(argv[++i]);
        } else {
            throw UsageError{"unknown argument: " + a};
        }
    }
    if (!cfg.showHelp && !cfg.showVersion && cfg.mapPath.empty()) {
        throw UsageError{"--map <path> is required (try --help)"};
    }
    return cfg;
}

}  // namespace pae::cli
