#pragma once

#include <ostream>
#include <vector>

#include "pae/metrics/Benchmark.hpp"

namespace pae::metrics {

class Report {
public:
    explicit Report(std::vector<BenchmarkRun> runs);

    /// Plain numeric table — one row per (algo, heuristic).
    void printTable(std::ostream& os) const;

    /// Visual ASCII bar charts — one chart per metric, one bar per
    /// (algo, heuristic) row, normalized against the metric's max.
    /// When `color` is true, each algorithm gets a distinct ANSI
    /// colour; otherwise the output is pure ASCII (snapshot-safe).
    /// See docs/PERFORMANCE.md "CLI bar charts".
    void printCharts(std::ostream& os, bool color = true) const;

    void writeCsv(std::ostream& os)   const;
    void writeJson(std::ostream& os)  const;

private:
    std::vector<BenchmarkRun> runs_;
};

}  // namespace pae::metrics
