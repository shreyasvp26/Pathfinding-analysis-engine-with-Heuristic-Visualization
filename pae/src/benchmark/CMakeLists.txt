# `pae_benchmark` is the orchestration layer. It composes algorithms
# and heuristics from the registry, runs them N times on a grid, and
# prints / dumps the results. It sits ABOVE pae_algorithms /
# pae_factory in the dependency DAG.
add_library(pae_benchmark
    Benchmark.cpp
    Report.cpp
)
target_link_libraries(pae_benchmark
    PUBLIC  pae_headers pae_core pae_metrics
            pae_algorithms pae_heuristics pae_visualization pae_factory
    PRIVATE pae_warnings
)
