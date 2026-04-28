add_library(pae_heuristics
    Manhattan.cpp
    Euclidean.cpp
    Chebyshev.cpp
)
target_link_libraries(pae_heuristics
    PUBLIC  pae_headers pae_core
    PRIVATE pae_warnings
)
