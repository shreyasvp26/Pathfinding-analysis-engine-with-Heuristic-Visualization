#pragma once

#include <filesystem>
#include <string_view>

#include "pae/core/Grid.hpp"
#include "pae/io/Errors.hpp"

namespace pae::io {

/// ASCII grid loader.
///
/// Format:
///   - Header section (before dimensions): lines starting with '#' are comments.
///   - First non-comment line: "<width> <height>".
///   - Subsequent lines: exactly <height> rows of <width> characters each.
///     '#' in the body section is the obstacle cell (NOT a comment marker).
///   - Cell chars: '.' Empty, '#' Obstacle, 'S' Start, 'E' End,
///     '0'..'9' Empty with weight 1+digit (so '0' = weight 1, '9' = weight 10).
///
/// Validation (throws IoError on violation):
///   - Exactly one 'S', exactly one 'E'.
///   - Rectangular shape (every row == width).
///   - Only legal characters.
class GridLoader {
public:
    static core::Grid loadFromFile(const std::filesystem::path& path);
    static core::Grid loadFromString(std::string_view text);
};

}  // namespace pae::io
