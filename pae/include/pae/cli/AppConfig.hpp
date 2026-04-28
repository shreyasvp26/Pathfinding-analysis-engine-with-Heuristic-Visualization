#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "pae/visualization/VizMode.hpp"

namespace pae::cli {

struct AppConfig {
    std::filesystem::path mapPath;
    std::string           algo      = "astar";
    std::string           heuristic = "manhattan";
    viz::VizMode          vizMode   = viz::VizMode::Step;
    bool                  diagonal  = false;
    bool                  color     = true;
    bool                  benchmark = false;
    bool                  showHelp  = false;
    bool                  showVersion = false;
    std::uint64_t         seed      = 0;
};

/// Throws pae::cli::UsageError on bad arguments.
AppConfig parseArgs(int argc, char** argv);

/// Returns the canonical --help text (also used by snapshot tests).
std::string helpText();

/// Returns "pae <Major>.<Minor>.<Patch>".
std::string versionText();

}  // namespace pae::cli
