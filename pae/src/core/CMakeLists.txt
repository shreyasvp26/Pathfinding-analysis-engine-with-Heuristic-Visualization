add_library(pae_core
    Grid.cpp
)
target_link_libraries(pae_core
    PUBLIC  pae_headers
    PRIVATE pae_warnings
)
