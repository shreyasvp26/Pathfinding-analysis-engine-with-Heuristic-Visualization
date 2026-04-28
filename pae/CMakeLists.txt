cmake_minimum_required(VERSION 3.20)

project(pae
    VERSION 0.1.0
    DESCRIPTION "Pathfinding Analysis Engine with Heuristic Visualization"
    LANGUAGES CXX
)

# ── Project options ───────────────────────────────────────────────────
option(PAE_BUILD_TESTS      "Build the Catch2 test suite"          ON)
option(PAE_BUILD_BENCHMARKS "Build the benchmark harness"          ON)
option(PAE_SANITIZERS       "Enable AddressSanitizer + UBSan"      OFF)
option(PAE_PERF_BUDGET      "Enable perf-budget tests (CI runner)" OFF)
option(PAE_NATIVE           "Use -march=native (non-portable)"     OFF)
option(PAE_TRUE_RSS         "Enable real-RSS metric collection"    OFF)
option(PAE_USE_GBENCH       "Pull in Google Benchmark (optional)"  OFF)

# ── Global config ─────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS         OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── Public header path (single source of truth) ──────────────────────
add_library(pae_headers INTERFACE)
target_include_directories(pae_headers INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# ── Compile flags helper ──────────────────────────────────────────────
add_library(pae_warnings INTERFACE)
if(MSVC)
    target_compile_options(pae_warnings INTERFACE /W4 /WX /permissive- /Zc:__cplusplus)
else()
    target_compile_options(pae_warnings INTERFACE
        -Wall -Wextra -Werror -Wpedantic
        -Wshadow -Wconversion -Wsign-conversion -Wold-style-cast
    )
endif()

if(PAE_NATIVE AND NOT MSVC)
    target_compile_options(pae_warnings INTERFACE -march=native)
endif()

if(PAE_SANITIZERS AND NOT MSVC)
    target_compile_options(pae_warnings INTERFACE
        -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
    target_link_options(pae_warnings INTERFACE
        -fsanitize=address -fsanitize=undefined)
endif()

# ── Subdirectories (declared in dependency order) ─────────────────────
add_subdirectory(src/core)            # depends on:    -
add_subdirectory(src/io)              # depends on:    core
add_subdirectory(src/metrics)         # depends on:    core (carries Metrics POD + RSS probe)
add_subdirectory(src/visualization)   # depends on:    core, metrics
add_subdirectory(src/heuristics)      # depends on:    core
add_subdirectory(src/algorithms)      # depends on:    core, heuristics, metrics
add_subdirectory(src/factory)         # depends on:    algorithms, heuristics, visualization, metrics
add_subdirectory(src/benchmark)       # depends on:    everything above
add_subdirectory(src/cli)             # depends on:    everything (composition root)

# ── Main binary ───────────────────────────────────────────────────────
add_executable(pae src/cli/main.cpp)
target_link_libraries(pae
    PRIVATE
        pae_app           # transitively brings in everything else
        pae_warnings
)

# ── Tests ─────────────────────────────────────────────────────────────
if(PAE_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.4
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    add_subdirectory(tests)
endif()

# ── Benchmarks ────────────────────────────────────────────────────────
if(PAE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
