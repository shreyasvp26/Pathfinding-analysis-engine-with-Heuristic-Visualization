#include "pae/visualization/CliVisualizer.hpp"

#include <algorithm>
#include <chrono>

#include "pae/core/Grid.hpp"

namespace pae::viz {

namespace {

constexpr const char* RESET   = "\x1b[0m";
constexpr const char* DIM_CYN = "\x1b[2;36m";
constexpr const char* YELLOW  = "\x1b[33m";
constexpr const char* MAGENTA = "\x1b[1;35m";
constexpr const char* GREEN   = "\x1b[1;32m";
constexpr const char* RED     = "\x1b[1;31m";
constexpr const char* GRAY    = "\x1b[90m";
constexpr const char* CLEAR   = "\x1b[2J\x1b[H";

}  // namespace

CliVisualizer::CliVisualizer(CliConfig cfg, std::ostream& out)
    : cfg_(cfg), out_(out) {}

void CliVisualizer::onSearchStart(const core::Grid& grid) {
    grid_ = &grid;
    const auto V = static_cast<std::size_t>(grid.width()) *
                   static_cast<std::size_t>(grid.height());
    openSet_.assign(V, 0);
    closedSet_.assign(V, 0);
    finalPath_.clear();
    lastFrame_ = std::chrono::steady_clock::time_point{};
}

void CliVisualizer::onEnqueue(core::Coord c, double /*f*/) {
    if (!grid_) return;
    openSet_[static_cast<std::size_t>(grid_->toIndex(c))] = 1;
}

void CliVisualizer::onExpand(core::Coord c) {
    if (!grid_) return;
    const auto idx = static_cast<std::size_t>(grid_->toIndex(c));
    openSet_[idx]   = 0;
    closedSet_[idx] = 1;

    if (cfg_.mode != VizMode::Step) {
        return;
    }
    if (cfg_.fpsCap > 0) {
        const auto now    = std::chrono::steady_clock::now();
        const auto period = std::chrono::milliseconds{1000 / cfg_.fpsCap};
        if (now - lastFrame_ < period) {
            return;
        }
        lastFrame_ = now;
    }
    render();
}

void CliVisualizer::onPathFound(core::Span<const core::Coord> path) {
    finalPath_.assign(path.begin(), path.end());
    if (cfg_.mode == VizMode::Final || cfg_.mode == VizMode::Step) {
        render();
    }
}

void CliVisualizer::onSearchComplete(const metrics::Metrics&) {
    // Nothing here; App prints the metrics summary.
}

void CliVisualizer::render() const {
    if (!grid_) return;
    const auto W = grid_->width();
    const auto H = grid_->height();

    if (cfg_.clearTerm) {
        out_ << CLEAR;
    }

    std::vector<std::uint8_t> isPath(static_cast<std::size_t>(W * H), 0);
    for (const auto& c : finalPath_) {
        isPath[static_cast<std::size_t>(grid_->toIndex(c))] = 1;
    }

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            const auto idx = static_cast<std::size_t>(r * W + c);
            const auto cell = grid_->unchecked({r, c});
            const bool path   = isPath[idx]   == 1;
            const bool open   = openSet_[idx] == 1;
            const bool closed = closedSet_[idx] == 1;

            const char* color = nullptr;
            char        ch    = '.';
            switch (cell) {
                case core::Cell::Obstacle: color = GRAY;   ch = '#'; break;
                case core::Cell::Start:    color = GREEN;  ch = 'S'; break;
                case core::Cell::End:      color = RED;    ch = 'E'; break;
                case core::Cell::Empty:
                default:
                    if (path)        { color = MAGENTA;  ch = '+'; }
                    else if (open)   { color = YELLOW;   ch = 'o'; }
                    else if (closed) { color = DIM_CYN;  ch = '*'; }
                    else             { color = nullptr;  ch = '.'; }
                    break;
            }
            if (cfg_.color && color) {
                out_ << color << ch << RESET;
            } else {
                out_ << ch;
            }
        }
        out_ << '\n';
    }
}

}  // namespace pae::viz
