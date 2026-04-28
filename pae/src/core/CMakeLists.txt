add_library(pae_core
    Grid.cpp
)
target_include_directories(pae_core
    PUBLIC  ${CMAKE_SOURCE_DIR}/pae/include
)
target_link_libraries(pae_core PRIVATE pae_warnings)
