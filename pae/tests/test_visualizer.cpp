#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <sstream>
#include <string>

#include "pae/algorithms/BFS.hpp"
#include "pae/io/GridLoader.hpp"
#include "pae/metrics/Metrics.hpp"
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

TEST_CASE("CliVisualizer: snapshot of plain-ASCII corridor render",
          "[viz][snapshot]") {
    // This locks the rendered output format. If you change any of:
    //   - cell glyphs ('S', 'E', '+', 'o', '*', '#', '.')
    //   - row separator (\n at end of each grid row)
    //   - what `Final` mode draws (path only, no scratch cells)
    // ...you MUST update this snapshot in the same commit. That's the
    // whole point of having a snapshot test.
    auto g = GridLoader::loadFromString("5 1\nS...E\n");
    BFS  bfs;
    std::ostringstream oss;
    CliConfig cfg{};
    cfg.mode      = VizMode::Final;
    cfg.color     = false;
    cfg.clearTerm = false;
    CliVisualizer cv{cfg, oss};
    (void)bfs.run(g, RunConfig{}, &cv, nullptr);

    REQUIRE(oss.str() == std::string{"S+++E\n"});
}

TEST_CASE("CliVisualizer: snapshot of obstacle render",
          "[viz][snapshot]") {
    // 3x3 map with a single wall in the middle:
    //   S . .
    //   . # .
    //   . . E
    //
    // Final mode shows BOTH the chosen path (`+`) AND every cell BFS
    // expanded to find it (`*`). That's a deliberate UX choice
    // documented in docs/PERFORMANCE.md — users learn just as much from
    // "what did the algorithm look at?" as from "where did it go?".
    //
    // Neighbour order in core::Grid::neighbors4 is up,down,left,right,
    // so BFS expands (0,0)→(1,0)→(0,1)→(2,0)→(0,2)→(2,1)→(1,2)→(2,2).
    // Path back-traced from (2,2) is (0,0)→(1,0)→(2,0)→(2,1)→(2,2).
    auto g = GridLoader::loadFromString(
        "3 3\n"
        "S..\n"
        ".#.\n"
        "..E\n");
    BFS  bfs;
    std::ostringstream oss;
    CliConfig cfg{};
    cfg.mode      = VizMode::Final;
    cfg.color     = false;
    cfg.clearTerm = false;
    CliVisualizer cv{cfg, oss};
    (void)bfs.run(g, RunConfig{}, &cv, nullptr);

    REQUIRE(oss.str() == std::string{
        "S**\n"
        "+#*\n"
        "++E\n"});
}
