#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "pae/core/Cell.hpp"
#include "pae/core/Coord.hpp"

namespace pae::core {

/// 2D grid stored row-major for cache locality.
///
/// Invariants (enforced in constructor):
///   1. cells.size() == width * height
///   2. weights.empty() OR weights.size() == cells.size()
///   3. cells[toIndex(start)] == Cell::Start
///   4. cells[toIndex(end)]   == Cell::End
///   5. start != end
///
/// Immutable after construction. Safe to share across threads.
class Grid {
public:
    Grid(std::int32_t width,
         std::int32_t height,
         std::vector<Cell> cells,
         std::vector<std::int32_t> weights,
         Coord start,
         Coord end);

    [[nodiscard]] std::int32_t width()  const noexcept { return width_; }
    [[nodiscard]] std::int32_t height() const noexcept { return height_; }
    [[nodiscard]] Coord        start()  const noexcept { return start_; }
    [[nodiscard]] Coord        end()    const noexcept { return end_; }

    [[nodiscard]] bool inBounds(Coord c) const noexcept;

    /// Bounds-checked accessor; throws std::out_of_range on miss.
    [[nodiscard]] Cell at(Coord c) const;

    /// Unchecked accessor; caller must have already verified inBounds.
    [[nodiscard]] Cell unchecked(Coord c) const noexcept;

    /// Cell weight; 1 if no weight buffer is attached.
    [[nodiscard]] std::int32_t weight(Coord c) const noexcept;

    /// Writes neighbours into `out`; sets `count` to the number written.
    /// Excludes off-grid cells but NOT obstacles (the algorithm filters).
    void neighbors4(Coord c, std::array<Coord, 4>& out, int& count) const noexcept;
    void neighbors8(Coord c, std::array<Coord, 8>& out, int& count) const noexcept;

    [[nodiscard]] std::int32_t toIndex(Coord c) const noexcept {
        return c.row * width_ + c.col;
    }
    [[nodiscard]] Coord toCoord(std::int32_t idx) const noexcept {
        return Coord{idx / width_, idx % width_};
    }

private:
    std::int32_t              width_;
    std::int32_t              height_;
    std::vector<Cell>         cells_;
    std::vector<std::int32_t> weights_;
    Coord                     start_;
    Coord                     end_;
};

}  // namespace pae::core
