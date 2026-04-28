add_library(pae_factory
    register_default.cpp
)
target_include_directories(pae_factory
    PUBLIC ${CMAKE_SOURCE_DIR}/pae/include
)
target_link_libraries(pae_factory
    PUBLIC  pae_algorithms pae_heuristics pae_visualization pae_metrics
    PRIVATE pae_warnings
)
