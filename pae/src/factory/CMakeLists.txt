add_library(pae_factory
    register_default.cpp
)
target_link_libraries(pae_factory
    PUBLIC  pae_headers pae_algorithms pae_heuristics pae_visualization pae_metrics
    PRIVATE pae_warnings
)
