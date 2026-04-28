add_library(pae_algorithms
    AStar.cpp
    Dijkstra.cpp
    BFS.cpp
    internal/Reconstruct.cpp
)
target_include_directories(pae_algorithms
    PUBLIC  ${CMAKE_SOURCE_DIR}/pae/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(pae_algorithms
    PUBLIC  pae_core pae_heuristics pae_metrics
    PRIVATE pae_warnings
)
