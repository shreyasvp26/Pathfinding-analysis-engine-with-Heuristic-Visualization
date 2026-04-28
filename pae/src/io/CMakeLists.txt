add_library(pae_io
    GridLoader.cpp
)
target_link_libraries(pae_io
    PUBLIC  pae_headers pae_core
    PRIVATE pae_warnings
)
