#pragma once

#include "pae/cli/AppConfig.hpp"

namespace pae::cli {

class App {
public:
    explicit App(AppConfig cfg) : cfg_(std::move(cfg)) {}

    /// Returns process exit code: 0 on success, 1 on user error,
    /// 2 on engine error.
    int run();

private:
    AppConfig cfg_;
};

}  // namespace pae::cli
