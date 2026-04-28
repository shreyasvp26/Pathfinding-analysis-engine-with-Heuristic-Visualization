#include <catch2/catch_test_macros.hpp>

#include <initializer_list>
#include <vector>

#include "pae/cli/AppConfig.hpp"
#include "pae/io/Errors.hpp"

using pae::cli::AppConfig;
using pae::cli::parseArgs;
using pae::cli::UsageError;

namespace {

AppConfig parse(std::initializer_list<const char*> args) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (auto* a : args) argv.push_back(const_cast<char*>(a));
    return parseArgs(static_cast<int>(argv.size()), argv.data());
}

}  // namespace

// NOTE: test names must not begin with '-' or Catch2's CLI parser
// consumes them when ctest invokes the binary. That's why these names
// are descriptive prose, not flag mirrors.

TEST_CASE("CLI: help flag sets showHelp", "[cli]") {
    auto cfg = parse({"pae", "--help"});
    REQUIRE(cfg.showHelp);
}

TEST_CASE("CLI: version flag sets showVersion", "[cli]") {
    auto cfg = parse({"pae", "--version"});
    REQUIRE(cfg.showVersion);
}

TEST_CASE("CLI: missing map argument throws UsageError", "[cli]") {
    REQUIRE_THROWS_AS(parse({"pae"}), UsageError);
}

TEST_CASE("CLI: standard map+algo+heuristic combo parses", "[cli]") {
    auto cfg = parse({"pae", "--map", "x.txt", "--algo", "dijkstra", "--visualize", "none"});
    REQUIRE(cfg.mapPath  == "x.txt");
    REQUIRE(cfg.algo     == "dijkstra");
    REQUIRE(cfg.vizMode  == pae::viz::VizMode::None);
}

TEST_CASE("CLI: A* with custom heuristic parses", "[cli]") {
    auto cfg = parse({"pae", "--map", "m.txt", "--algo", "astar", "--heuristic", "euclidean"});
    REQUIRE(cfg.algo      == "astar");
    REQUIRE(cfg.heuristic == "euclidean");
}

TEST_CASE("CLI: --diagonal and --no-color toggles parse", "[cli]") {
    auto cfg = parse({"pae", "--map", "m.txt", "--diagonal", "--no-color"});
    REQUIRE(cfg.diagonal);
    REQUIRE_FALSE(cfg.color);
}

TEST_CASE("CLI: --benchmark is a top-level mode", "[cli]") {
    auto cfg = parse({"pae", "--map", "m.txt", "--benchmark"});
    REQUIRE(cfg.benchmark);
}

TEST_CASE("CLI: unknown argument throws UsageError", "[cli]") {
    REQUIRE_THROWS_AS(parse({"pae", "--map", "x.txt", "--foo"}), UsageError);
}

TEST_CASE("CLI: missing value after --algo throws UsageError", "[cli]") {
    REQUIRE_THROWS_AS(parse({"pae", "--map", "x.txt", "--algo"}), UsageError);
}

TEST_CASE("CLI: unknown --visualize value throws UsageError", "[cli]") {
    REQUIRE_THROWS_AS(parse({"pae", "--map", "x.txt", "--visualize", "live"}), UsageError);
}
