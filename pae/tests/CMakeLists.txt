include(Catch)

set(PAE_TEST_SOURCES
    test_smoke.cpp
    test_grid.cpp
    test_grid_loader.cpp
    test_manhattan.cpp
    test_euclidean.cpp
    test_chebyshev.cpp
    test_octile.cpp
    test_bfs.cpp
    test_dijkstra.cpp
    test_astar.cpp
    test_cross_algorithm.cpp
    test_visualizer.cpp
    test_charts.cpp
    test_registry.cpp
    test_cli.cpp
)

# Perf-budget tests (F-404): only compiled with -DPAE_PERF_BUDGET=ON
# (see `--preset perf`). They exercise Benchmark::sweep on the larger
# maps and assert order-of-magnitude wall-time and path-cost budgets.
# Catch2 tags `[perf]` are auto-promoted to ctest LABELS by
# `catch_discover_tests`, so the `perf-budget` CI job filters on them
# via `ctest -L perf`.
if(PAE_PERF_BUDGET)
    list(APPEND PAE_TEST_SOURCES test_perf_budget.cpp)
endif()

add_executable(pae_tests ${PAE_TEST_SOURCES})
target_include_directories(pae_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pae_tests
    PRIVATE
        Catch2::Catch2WithMain
        pae_headers
        pae_core
        pae_io
        pae_heuristics
        pae_algorithms
        pae_visualization
        pae_metrics
        pae_benchmark
        pae_factory
        pae_app
        pae_warnings
)

# Discover every test EXCEPT the perf-budget ones. `~[perf]` is
# Catch2's "exclude tag" syntax. We re-register the perf tests below
# with an explicit `LABELS "perf"` property, because
# `catch_discover_tests` does not lift Catch2 tags into ctest labels
# (and the `perf-budget` CI job filters with `ctest -L perf`).
catch_discover_tests(pae_tests
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    TEST_SPEC          "~[perf]"
)

if(PAE_PERF_BUDGET)
    set(PAE_PERF_CASES
        "perf budget: maze_20x20 (every config under 50 ms median)"
        "perf budget: open_arena_50x50 (every config under 500 ms median)"
        "perf budget: optimality is preserved (every config agrees on path cost)"
    )
    foreach(_case IN LISTS PAE_PERF_CASES)
        add_test(
            NAME              "${_case}"
            COMMAND           pae_tests "${_case}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        )
        set_tests_properties("${_case}" PROPERTIES LABELS "perf")
    endforeach()
endif()
