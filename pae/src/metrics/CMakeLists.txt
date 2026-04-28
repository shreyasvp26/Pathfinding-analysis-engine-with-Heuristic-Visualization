# `pae_metrics` exposes the `Metrics` POD struct and the OS-level RSS
# probe (`maxResidentBytes`). Algorithms link this; they must NOT
# depend on the higher-level Benchmark / Report code (which would
# form a cycle because Benchmark uses the algorithm classes themselves).
#
# `Rss.cpp` is gated by the PAE_TRUE_RSS option (see top-level
# CMakeLists.txt). When OFF, the function returns 0 and algorithms
# fall back to the analytical working-set model.
add_library(pae_metrics
    Rss.cpp
)
target_link_libraries(pae_metrics
    PUBLIC  pae_headers pae_core
    PRIVATE pae_warnings
)

if(PAE_TRUE_RSS)
    target_compile_definitions(pae_metrics PUBLIC PAE_TRUE_RSS=1)
endif()
