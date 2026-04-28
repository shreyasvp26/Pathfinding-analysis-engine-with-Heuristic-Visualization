#include "pae/factory/Registry.hpp"

#include <memory>

#include "pae/algorithms/AStar.hpp"
#include "pae/algorithms/BFS.hpp"
#include "pae/algorithms/Dijkstra.hpp"
#include "pae/heuristics/Chebyshev.hpp"
#include "pae/heuristics/Euclidean.hpp"
#include "pae/heuristics/IHeuristic.hpp"
#include "pae/heuristics/Manhattan.hpp"

namespace pae::factory {

namespace {

// AStar is registered as a default-constructed sentinel in the
// Registry only for non-AStar algorithms. AStar requires a heuristic
// at construction; the App layer constructs it directly using a
// heuristic from Registry<IHeuristic>. For Registry<IPathfinder>,
// "astar" entry returns a placeholder that holds a Manhattan instance
// — App overrides it. (Documented contract; Benchmark::sweep follows
// the same pattern.)
class DefaultAStar : public algo::IPathfinder {
public:
    DefaultAStar() : real_(h_) {}
    algo::Result run(const core::Grid&     g,
                     const algo::RunConfig& cfg,
                     viz::IVisualizer*      viz,
                     metrics::Metrics*      m) const override {
        return real_.run(g, cfg, viz, m);
    }
    std::string_view name() const noexcept override { return "astar"; }

private:
    heur::Manhattan h_;
    algo::AStar     real_;
};

}  // namespace

void registerAll() {
    auto& heurReg = Registry<heur::IHeuristic>::instance();
    heurReg.reg("manhattan", [] { return std::make_unique<heur::Manhattan>(); });
    heurReg.reg("euclidean", [] { return std::make_unique<heur::Euclidean>(); });
    heurReg.reg("chebyshev", [] { return std::make_unique<heur::Chebyshev>(); });

    auto& algoReg = Registry<algo::IPathfinder>::instance();
    algoReg.reg("astar",    [] { return std::make_unique<DefaultAStar>(); });
    algoReg.reg("dijkstra", [] { return std::make_unique<algo::Dijkstra>(); });
    algoReg.reg("bfs",      [] { return std::make_unique<algo::BFS>(); });
}

}  // namespace pae::factory
