add_library(pae_visualization
    CliVisualizer.cpp
)
target_link_libraries(pae_visualization
    PUBLIC  pae_headers pae_core pae_metrics
    PRIVATE pae_warnings
)
