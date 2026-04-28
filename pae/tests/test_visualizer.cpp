#include <catch2/catch_test_macros.hpp>

#include <sstream>

#include "pae/algorithms/BFS.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/visualization/CliVisualizer.hpp"
#include "pae/visualization/NullVisualizer.hpp"

using pae::algo::BFS;
using pae::algo::RunConfig;
using pae::io::GridLoader;
using pae::viz::CliConfig;
using pae::viz::CliVisualizer;
using pae::viz::NullVisualizer;
using pae::viz::VizMode;

TEST_CASE("NullVisualizer produces no output", "[viz]") {
    auto g = GridLoader::loadFromString("5 1\nS...E\n");
    BFS  bfs;
    NullVisualizer nv;
    (void)bfs.run(g, RunConfig{}, &nv, nullptr);
    SUCCEED();   // if the call returned without crashing or printing, we're good
}

TEST_CASE("CliVisualizer Final mode renders a path", "[viz]") {
    auto g = GridLoader::loadFromString("5 1\nS...E\n");
    BFS  bfs;
    std::ostringstream oss;
    CliConfig cfg{};
    cfg.mode      = VizMode::Final;
    cfg.color     = false;
    cfg.clearTerm = false;
    CliVisualizer cv{cfg, oss};
    (void)bfs.run(g, RunConfig{}, &cv, nullptr);
    const auto out = oss.str();
    // The corridor is "S+++E\n" (5 chars + newline). Plus 'S' and 'E' colored cells.
    REQUIRE(out.find('S') != std::string::npos);
    REQUIRE(out.find('E') != std::string::npos);
    REQUIRE(out.find('+') != std::string::npos);
}
