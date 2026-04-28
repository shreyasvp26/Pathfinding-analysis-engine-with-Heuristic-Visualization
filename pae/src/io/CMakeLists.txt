add_library(pae_io
    GridLoader.cpp
)
target_include_directories(pae_io
    PUBLIC ${CMAKE_SOURCE_DIR}/pae/include
)
target_link_libraries(pae_io
    PUBLIC  pae_core
    PRIVATE pae_warnings
)
