#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("--help short-circuits", "[cli]") {
    auto cfg = parse({"pae", "--help"});
    REQUIRE(cfg.showHelp);
}

TEST_CASE("--map is required if not help/version", "[cli]") {
    REQUIRE_THROWS_AS(parse({"pae"}), UsageError);
}

TEST_CASE("Standard map + algo + heuristic parses", "[cli]") {
    auto cfg = parse({"pae", "--map", "x.txt", "--algo", "dijkstra", "--visualize", "none"});
    REQUIRE(cfg.mapPath  == "x.txt");
    REQUIRE(cfg.algo     == "dijkstra");
    REQUIRE(cfg.vizMode  == pae::viz::VizMode::None);
}

TEST_CASE("Unknown arg throws UsageError", "[cli]") {
    REQUIRE_THROWS_AS(parse({"pae", "--map", "x.txt", "--foo"}), UsageError);
}
