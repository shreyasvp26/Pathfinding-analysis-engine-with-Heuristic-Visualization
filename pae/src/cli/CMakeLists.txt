add_library(pae_app
    AppConfig.cpp
    App.cpp
)
target_include_directories(pae_app
    PUBLIC ${CMAKE_SOURCE_DIR}/pae/include
)
target_link_libraries(pae_app
    PUBLIC
        pae_core
        pae_io
        pae_algorithms
        pae_heuristics
        pae_visualization
        pae_metrics
        pae_factory
    PRIVATE pae_warnings
)
