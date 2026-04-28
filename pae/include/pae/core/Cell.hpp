#pragma once

#include <cstdint>

namespace pae::core {

/// Cell type for `Grid`. Packed for cache density.
enum class Cell : std::uint8_t {
    Empty    = 0,
    Obstacle = 1,
    Start    = 2,
    End      = 3,
};

}  // namespace pae::core
