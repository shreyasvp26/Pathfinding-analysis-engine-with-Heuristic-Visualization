add_library(pae_visualization
    CliVisualizer.cpp
)
target_include_directories(pae_visualization
    PUBLIC ${CMAKE_SOURCE_DIR}/pae/include
)
target_link_libraries(pae_visualization
    PUBLIC  pae_core pae_metrics
    PRIVATE pae_warnings
)
