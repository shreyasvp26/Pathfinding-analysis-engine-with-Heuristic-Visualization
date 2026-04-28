cmake_minimum_required(VERSION 3.20)

# Top-level repo CMake — delegates the real build to pae/.
# This file exists so `cmake -S . -B build` works from the repo root,
# matching the workflow used in CI and in the sibling SSM/Jyotish projects.

project(pathfinding-analysis-engine
    VERSION 0.1.0
    DESCRIPTION "Pathfinding Analysis Engine with Heuristic Visualization"
    LANGUAGES CXX
)

# All real build logic lives inside pae/.
add_subdirectory(pae)
