#include "pae/io/GridLoader.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace pae::io {

using core::Cell;
using core::Coord;
using core::Grid;

namespace {

[[nodiscard]] std::string readEntireFile(const std::filesystem::path& p) {
    std::ifstream f{p};
    if (!f) {
        throw IoError{"GridLoader: cannot open file: " + p.string()};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

}  // namespace

Grid GridLoader::loadFromFile(const std::filesystem::path& path) {
    return loadFromString(readEntireFile(path));
}

Grid GridLoader::loadFromString(std::string_view text) {
    // Skip comment lines and capture the first "<width> <height>" line.
    std::istringstream in{std::string{text}};
    std::string        line;

    int width  = 0;
    int height = 0;
    bool gotDims = false;

    // Pre-header lines: '#' is a comment marker. Empty lines are skipped.
    // Once dimensions are parsed, '#' becomes the obstacle cell — never a comment.
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream ls{line};
        if (!(ls >> width >> height) || width <= 0 || height <= 0) {
            throw IoError{"GridLoader: expected '<width> <height>' as first non-comment line"};
        }
        gotDims = true;
        break;
    }
    if (!gotDims) {
        throw IoError{"GridLoader: missing dimensions header"};
    }

    std::vector<Cell>         cells;
    std::vector<std::int32_t> weights;
    cells.reserve(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));

    Coord start{-1, -1};
    Coord end{-1, -1};
    bool  weighted = false;
    int   row      = 0;

    while (std::getline(in, line)) {
        // After dimensions, only blank lines are skipped. '#' is now the
        // obstacle character (cells of full obstacle rows would be lost otherwise).
        if (line.empty()) {
            continue;
        }
        if (row >= height) {
            throw IoError{"GridLoader: more rows than declared height"};
        }
        if (static_cast<int>(line.size()) != width) {
            throw IoError{"GridLoader: row " + std::to_string(row) +
                          " has length " + std::to_string(line.size()) +
                          ", expected " + std::to_string(width)};
        }
        for (int col = 0; col < width; ++col) {
            const char ch = line[static_cast<std::size_t>(col)];
            switch (ch) {
                case '.':
                    cells.push_back(Cell::Empty);
                    weights.push_back(1);
                    break;
                case '#':
                    cells.push_back(Cell::Obstacle);
                    weights.push_back(1);
                    break;
                case 'S':
                    if (start.row != -1) {
                        throw IoError{"GridLoader: multiple Start cells"};
                    }
                    start = Coord{row, col};
                    cells.push_back(Cell::Start);
                    weights.push_back(1);
                    break;
                case 'E':
                    if (end.row != -1) {
                        throw IoError{"GridLoader: multiple End cells"};
                    }
                    end = Coord{row, col};
                    cells.push_back(Cell::End);
                    weights.push_back(1);
                    break;
                default:
                    if (ch >= '0' && ch <= '9') {
                        cells.push_back(Cell::Empty);
                        weights.push_back(1 + (ch - '0'));
                        weighted = true;
                    } else {
                        throw IoError{std::string{"GridLoader: invalid character '"} + ch +
                                      "' at row " + std::to_string(row) +
                                      " col "    + std::to_string(col)};
                    }
                    break;
            }
        }
        ++row;
    }

    if (row != height) {
        throw IoError{"GridLoader: fewer rows than declared height"};
    }
    if (start.row == -1) {
        throw IoError{"GridLoader: no Start cell"};
    }
    if (end.row == -1) {
        throw IoError{"GridLoader: no End cell"};
    }

    if (!weighted) {
        weights.clear();
    }
    return Grid{width, height, std::move(cells), std::move(weights), start, end};
}

}  // namespace pae::io
