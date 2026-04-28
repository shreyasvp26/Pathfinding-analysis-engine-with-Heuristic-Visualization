# `pae_metrics` is a header-only INTERFACE library exposing the
# `Metrics` struct. Algorithms link this; they must NOT depend on
# the higher-level Benchmark / Report code (which would form a cycle
# because Benchmark uses the algorithm classes themselves).
add_library(pae_metrics INTERFACE)
target_link_libraries(pae_metrics
    INTERFACE pae_headers pae_core
)
