#pragma once

#include <ostream>
#include <vector>

#include "pae/metrics/Benchmark.hpp"

namespace pae::metrics {

class Report {
public:
    explicit Report(std::vector<BenchmarkRun> runs);

    void printTable(std::ostream& os) const;
    void writeCsv(std::ostream& os)   const;
    void writeJson(std::ostream& os)  const;

private:
    std::vector<BenchmarkRun> runs_;
};

}  // namespace pae::metrics
