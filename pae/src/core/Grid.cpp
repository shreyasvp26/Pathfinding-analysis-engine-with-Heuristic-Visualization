#include "pae/core/Grid.hpp"

#include <stdexcept>
#include <utility>

namespace pae::core {

Grid::Grid(std::int32_t width,
           std::int32_t height,
           std::vector<Cell> cells,
           std::vector<std::int32_t> weights,
           Coord start,
           Coord end)
    : width_(width),
      height_(height),
      cells_(std::move(cells)),
      weights_(std::move(weights)),
      start_(start),
      end_(end) {
    if (width_ <= 0 || height_ <= 0) {
        throw std::invalid_argument{"Grid dimensions must be positive"};
    }
    const auto expected = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);
    if (cells_.size() != expected) {
        throw std::invalid_argument{"cells.size() must equal width*height"};
    }
    if (!weights_.empty() && weights_.size() != cells_.size()) {
        throw std::invalid_argument{"weights.size() must be 0 or width*height"};
    }
    if (!inBounds(start_) || !inBounds(end_) || start_ == end_) {
        throw std::invalid_argument{"start/end out of bounds or equal"};
    }
    if (cells_[static_cast<std::size_t>(toIndex(start_))] != Cell::Start) {
        throw std::invalid_argument{"start cell must be Cell::Start"};
    }
    if (cells_[static_cast<std::size_t>(toIndex(end_))] != Cell::End) {
        throw std::invalid_argument{"end cell must be Cell::End"};
    }
}

bool Grid::inBounds(Coord c) const noexcept {
    return c.row >= 0 && c.row < height_ && c.col >= 0 && c.col < width_;
}

Cell Grid::at(Coord c) const {
    if (!inBounds(c)) {
        throw std::out_of_range{"Grid::at: out of bounds"};
    }
    return cells_[static_cast<std::size_t>(toIndex(c))];
}

Cell Grid::unchecked(Coord c) const noexcept {
    return cells_[static_cast<std::size_t>(toIndex(c))];
}

std::int32_t Grid::weight(Coord c) const noexcept {
    if (weights_.empty()) {
        return 1;
    }
    return weights_[static_cast<std::size_t>(toIndex(c))];
}

void Grid::neighbors4(Coord c, std::array<Coord, 4>& out, int& count) const noexcept {
    count = 0;
    constexpr int dr[4] = {-1, 1, 0, 0};
    constexpr int dc[4] = {0, 0, -1, 1};
    for (int i = 0; i < 4; ++i) {
        const Coord n{c.row + dr[i], c.col + dc[i]};
        if (inBounds(n)) {
            out[static_cast<std::size_t>(count++)] = n;
        }
    }
}

void Grid::neighbors8(Coord c, std::array<Coord, 8>& out, int& count) const noexcept {
    count = 0;
    constexpr int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    constexpr int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    for (int i = 0; i < 8; ++i) {
        const Coord n{c.row + dr[i], c.col + dc[i]};
        if (inBounds(n)) {
            out[static_cast<std::size_t>(count++)] = n;
        }
    }
}

}  // namespace pae::core
