#pragma once

#include <cstdint>

namespace pae::viz {

enum class VizMode : std::uint8_t {
    None  = 0,
    Final = 1,
    Step  = 2,
};

}  // namespace pae::viz
