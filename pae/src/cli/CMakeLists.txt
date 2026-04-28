add_library(pae_app
    AppConfig.cpp
    App.cpp
)
target_link_libraries(pae_app
    PUBLIC
        pae_headers
        pae_core
        pae_io
        pae_algorithms
        pae_heuristics
        pae_visualization
        pae_metrics
        pae_benchmark
        pae_factory
    PRIVATE pae_warnings
)
