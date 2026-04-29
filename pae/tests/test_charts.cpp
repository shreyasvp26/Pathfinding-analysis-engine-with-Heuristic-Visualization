// Snapshot test for Report::printCharts.
//
// We fabricate a tiny, fully-known BenchmarkRun set and lock the
// plain-ASCII (no-color) bar-chart output. This is deliberately
// strict: every column must line up, the scale must be normalised
// per chart, and the metric labels must match what the README +
// docs/PERFORMANCE.md describe.
//
// To regenerate the expected string after an intentional format
// change: run the test, copy the actual block from the failure
// output, and paste it back below as the new snapshot.

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>
#include <vector>

#include "pae/metrics/Report.hpp"

using pae::metrics::BenchmarkRun;
using pae::metrics::Metrics;
using pae::metrics::Report;

namespace {

BenchmarkRun makeRun(std::string algo, std::string heur,
                     std::int64_t wallUs, std::int64_t expanded,
                     std::int64_t pathCost, std::int64_t bytes,
                     int rep = 0) {
    BenchmarkRun r{};
    r.algoName      = std::move(algo);
    r.heuristicName = std::move(heur);
    r.repIndex      = rep;
    r.metrics.wallMicros      = wallUs;
    r.metrics.nodesExpanded   = expanded;
    r.metrics.nodesEnqueued   = expanded;
    r.metrics.pathLength      = 10;
    r.metrics.pathCost        = pathCost;
    r.metrics.approxPeakBytes = bytes;
    return r;
}

}  // namespace

TEST_CASE("Report::printCharts: snapshot of plain-ASCII bar charts",
          "[viz][report][snapshot]") {
    // A* with manhattan is the cheapest; BFS the most expanded.
    std::vector<BenchmarkRun> runs;
    runs.push_back(makeRun("astar",    "manhattan", 100, 200, 50, 4000));
    runs.push_back(makeRun("bfs",      "",          200, 400, 50, 2000));
    runs.push_back(makeRun("dijkstra", "",          150, 300, 50, 3000));

    Report report{std::move(runs)};
    std::ostringstream os;
    report.printCharts(os, /*color=*/false);

    const std::string expected =
        "\n=== visual benchmark (bars normalised per chart) ===\n"
        "\n"
        "wall-time median (lower is faster)\n"
        "  astar manhattan        |####################                    |        100 us\n"
        "  bfs -                  |########################################|        200 us\n"
        "  dijkstra -             |##############################          |        150 us\n"
        "\n"
        "nodes expanded (lower = more directed search)\n"
        "  astar manhattan        |####################                    |        200   \n"
        "  bfs -                  |########################################|        400   \n"
        "  dijkstra -             |##############################          |        300   \n"
        "\n"
        "path cost (must be equal for optimal algorithms)\n"
        "  astar manhattan        |########################################|         50   \n"
        "  bfs -                  |########################################|         50   \n"
        "  dijkstra -             |########################################|         50   \n"
        "\n"
        "approximate peak memory (bytes; lower is leaner)\n"
        "  astar manhattan        |########################################|       4000 B \n"
        "  bfs -                  |####################                    |       2000 B \n"
        "  dijkstra -             |##############################          |       3000 B \n"
        "\n";

    REQUIRE(os.str() == expected);
}

TEST_CASE("Report::printCharts: empty input does not crash and emits header",
          "[viz][report]") {
    Report report{{}};
    std::ostringstream os;
    REQUIRE_NOTHROW(report.printCharts(os, /*color=*/false));
    REQUIRE_FALSE(os.str().empty());
}

TEST_CASE("Report::printCharts: ANSI mode emits reset codes for every bar",
          "[viz][report]") {
    std::vector<BenchmarkRun> runs;
    runs.push_back(makeRun("astar", "octile", 100, 200, 50, 4000));
    runs.push_back(makeRun("bfs",   "",       200, 400, 50, 2000));

    Report report{std::move(runs)};
    std::ostringstream os;
    report.printCharts(os, /*color=*/true);

    const std::string out = os.str();
    REQUIRE(out.find("\x1b[1;32m") != std::string::npos);  // astar = green
    REQUIRE(out.find("\x1b[1;33m") != std::string::npos);  // bfs = yellow
    // Every coloured run must be followed by a reset somewhere later.
    REQUIRE(out.find("\x1b[0m")    != std::string::npos);
}
