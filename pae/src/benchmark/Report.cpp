#include "pae/metrics/Report.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <numeric>
#include <utility>
#include <vector>

namespace pae::metrics {

namespace {

struct Aggregate {
    std::int64_t medianMicros{0};
    std::int64_t p95Micros{0};
    std::int64_t minMicros{0};
    std::int64_t maxMicros{0};
    std::int64_t expanded{0};
    std::int64_t enqueued{0};
    std::int64_t pathLen{0};
    std::int64_t pathCost{0};
    std::int64_t approxBytes{0};
    std::int64_t rssDelta{0};
    int          samples{0};
};

Aggregate aggregate(const std::vector<BenchmarkRun>& sub) {
    Aggregate a{};
    if (sub.empty()) return a;
    std::vector<std::int64_t> us;
    us.reserve(sub.size());
    for (const auto& r : sub) {
        us.push_back(r.metrics.wallMicros);
    }
    std::sort(us.begin(), us.end());
    const auto n = us.size();
    a.medianMicros = us[n / 2];
    a.p95Micros    = us[static_cast<std::size_t>(static_cast<double>(n) * 0.95)];
    a.minMicros    = us.front();
    a.maxMicros    = us.back();
    a.expanded     = sub.front().metrics.nodesExpanded;
    a.enqueued     = sub.front().metrics.nodesEnqueued;
    a.pathLen      = sub.front().metrics.pathLength;
    a.pathCost     = sub.front().metrics.pathCost;
    a.approxBytes  = sub.front().metrics.approxPeakBytes;
    a.rssDelta     = sub.front().metrics.rssDeltaBytes;
    a.samples      = static_cast<int>(sub.size());
    return a;
}

}  // namespace

Report::Report(std::vector<BenchmarkRun> runs) : runs_(std::move(runs)) {}

void Report::printTable(std::ostream& os) const {
    using Key = std::pair<std::string, std::string>;
    std::map<Key, std::vector<BenchmarkRun>> grouped;
    for (const auto& r : runs_) {
        grouped[{r.algoName, r.heuristicName}].push_back(r);
    }
    os << "algorithm   heuristic    expanded    wall_us(med)   wall_us(p95)   path_len   path_cost   memory_B\n";
    os << "---------   ----------   ---------   ------------   ------------   --------   ---------   --------\n";
    for (auto& [k, rs] : grouped) {
        const auto a = aggregate(rs);
        os << std::left << std::setw(12) << k.first
           << std::setw(13) << (k.second.empty() ? "-" : k.second)
           << std::right << std::setw(10) << a.expanded << "   "
           << std::setw(13) << a.medianMicros << "   "
           << std::setw(13) << a.p95Micros << "   "
           << std::setw(8)  << a.pathLen << "   "
           << std::setw(9)  << a.pathCost << "   "
           << std::setw(8)  << a.approxBytes
           << "\n";
    }
}

void Report::writeCsv(std::ostream& os) const {
    os << "algorithm,heuristic,rep,expanded,enqueued,wall_us,path_len,path_cost,approx_peak_bytes,rss_delta_bytes\n";
    for (const auto& r : runs_) {
        os << r.algoName << ',' << r.heuristicName << ','
           << r.repIndex << ','
           << r.metrics.nodesExpanded << ','
           << r.metrics.nodesEnqueued << ','
           << r.metrics.wallMicros << ','
           << r.metrics.pathLength << ','
           << r.metrics.pathCost << ','
           << r.metrics.approxPeakBytes << ','
           << r.metrics.rssDeltaBytes << '\n';
    }
}

void Report::writeJson(std::ostream& os) const {
    using Key = std::pair<std::string, std::string>;
    std::map<Key, std::vector<BenchmarkRun>> grouped;
    for (const auto& r : runs_) {
        grouped[{r.algoName, r.heuristicName}].push_back(r);
    }
    os << "{\n  \"runs\": [\n";
    bool firstGroup = true;
    for (auto& [k, rs] : grouped) {
        const auto a = aggregate(rs);
        if (!firstGroup) os << ",\n";
        firstGroup = false;
        os << "    {\n";
        os << "      \"algorithm\": \"" << k.first << "\",\n";
        os << "      \"heuristic\": \"" << k.second << "\",\n";
        os << "      \"samples\": "     << a.samples << ",\n";
        os << "      \"metrics\": {\n";
        os << "        \"expanded\": "  << a.expanded << ",\n";
        os << "        \"enqueued\": "  << a.enqueued << ",\n";
        os << "        \"path_len\": "  << a.pathLen << ",\n";
        os << "        \"path_cost\": " << a.pathCost << ",\n";
        os << "        \"approx_peak_bytes\": " << a.approxBytes << ",\n";
        os << "        \"rss_delta_bytes\": "    << a.rssDelta << ",\n";
        os << "        \"wall_us\": { \"median\": " << a.medianMicros
           << ", \"p95\": " << a.p95Micros
           << ", \"min\": " << a.minMicros
           << ", \"max\": " << a.maxMicros << " }\n";
        os << "      }\n    }";
    }
    os << "\n  ]\n}\n";
}

}  // namespace pae::metrics
