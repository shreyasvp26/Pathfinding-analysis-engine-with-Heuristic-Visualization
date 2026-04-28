add_library(pae_heuristics
    Manhattan.cpp
    Euclidean.cpp
    Chebyshev.cpp
)
target_include_directories(pae_heuristics
    PUBLIC ${CMAKE_SOURCE_DIR}/pae/include
)
target_link_libraries(pae_heuristics
    PUBLIC  pae_core
    PRIVATE pae_warnings
)
