include(Catch)

add_executable(pae_tests
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
    test_registry.cpp
    test_cli.cpp
)
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

catch_discover_tests(pae_tests
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
)
