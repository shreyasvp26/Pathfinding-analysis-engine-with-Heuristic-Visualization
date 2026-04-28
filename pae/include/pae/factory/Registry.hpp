#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "pae/io/Errors.hpp"

namespace pae::factory {

/// Generic name -> factory map for an interface T.
///
/// Used for IPathfinder and IHeuristic. The composition root
/// (`registerAll()` in `register_default.cpp`) populates it.
template <typename T>
class Registry {
public:
    using Factory = std::function<std::unique_ptr<T>()>;

    static Registry& instance() {
        static Registry s;
        return s;
    }

    /// Registers (or overwrites) a factory under `name`. Idempotent —
    /// repeated `registerAll()` calls (e.g. across test cases) replace
    /// rather than silently keeping stale closures.
    void reg(std::string_view name, Factory f) {
        map_[std::string{name}] = std::move(f);
    }

    [[nodiscard]] std::unique_ptr<T> create(std::string_view name) const {
        const auto it = map_.find(std::string{name});
        if (it == map_.end()) {
            throw UnknownNameError{"unknown name: " + std::string{name}};
        }
        return it->second();
    }

    [[nodiscard]] std::vector<std::string> names() const {
        std::vector<std::string> ns;
        ns.reserve(map_.size());
        for (const auto& [k, _] : map_) {
            ns.push_back(k);
        }
        return ns;
    }

private:
    Registry() = default;
    std::unordered_map<std::string, Factory> map_;
};

/// Composition root: registers every built-in algorithm + heuristic.
/// Must be called once at process start (in `main.cpp`).
void registerAll();

}  // namespace pae::factory
