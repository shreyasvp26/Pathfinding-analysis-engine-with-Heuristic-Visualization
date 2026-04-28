#include <catch2/catch_test_macros.hpp>

#include <array>
#include <vector>

#include "pae/core/Grid.hpp"

using pae::core::Cell;
using pae::core::Coord;
using pae::core::Grid;

namespace {

Grid makeTinyGrid() {
    // 3x3, S at (0,0), E at (0,2), no obstacles.
    std::vector<Cell> cells{
        Cell::Start, Cell::Empty, Cell::End,
        Cell::Empty, Cell::Empty, Cell::Empty,
        Cell::Empty, Cell::Empty, Cell::Empty,
    };
    return Grid{3, 3, std::move(cells), {}, Coord{0, 0}, Coord{0, 2}};
}

}  // namespace

TEST_CASE("Grid invariants enforced", "[grid][core]") {
    REQUIRE_NOTHROW(makeTinyGrid());
}

TEST_CASE("Grid::inBounds correct on 3x3", "[grid][core]") {
    auto g = makeTinyGrid();
    REQUIRE(g.inBounds({0, 0}));
    REQUIRE(g.inBounds({2, 2}));
    REQUIRE_FALSE(g.inBounds({-1, 0}));
    REQUIRE_FALSE(g.inBounds({0, 3}));
}

TEST_CASE("Grid::neighbors4 counts edge correctly", "[grid][core]") {
    auto                  g = makeTinyGrid();
    std::array<Coord, 4>  out{};
    int                   count{};
    g.neighbors4({0, 0}, out, count);
    REQUIRE(count == 2);    // (1,0) and (0,1)
    g.neighbors4({1, 1}, out, count);
    REQUIRE(count == 4);
}

TEST_CASE("Grid::weight defaults to 1 with no weight buffer", "[grid][core]") {
    auto g = makeTinyGrid();
    REQUIRE(g.weight({0, 0}) == 1);
    REQUIRE(g.weight({2, 2}) == 1);
}
