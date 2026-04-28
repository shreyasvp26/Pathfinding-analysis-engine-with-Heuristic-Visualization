#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace pae::core {

/// 2D integer grid coordinate. Trivially copyable, hashable, value type.
struct Coord {
    std::int32_t row{};
    std::int32_t col{};

    constexpr Coord() = default;
    constexpr Coord(std::int32_t r, std::int32_t c) : row(r), col(c) {}

    constexpr bool operator==(Coord o) const noexcept {
        return row == o.row && col == o.col;
    }
    constexpr bool operator!=(Coord o) const noexcept { return !(*this == o); }
};

struct CoordHash {
    std::size_t operator()(Coord c) const noexcept {
        const auto h1 = static_cast<std::uint64_t>(static_cast<std::uint32_t>(c.row));
        const auto h2 = static_cast<std::uint64_t>(static_cast<std::uint32_t>(c.col));
        return std::hash<std::uint64_t>{}((h1 << 32) ^ h2);
    }
};

}  // namespace pae::core
