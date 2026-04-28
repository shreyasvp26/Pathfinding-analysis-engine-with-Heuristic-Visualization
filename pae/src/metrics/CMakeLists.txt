add_library(pae_metrics
    Benchmark.cpp
    Report.cpp
)
target_include_directories(pae_metrics
    PUBLIC ${CMAKE_SOURCE_DIR}/pae/include
)
target_link_libraries(pae_metrics
    PUBLIC  pae_core
    PRIVATE pae_warnings
)
