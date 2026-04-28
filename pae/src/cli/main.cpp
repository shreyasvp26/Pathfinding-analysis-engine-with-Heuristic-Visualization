#include <exception>
#include <iostream>

#include "pae/cli/App.hpp"
#include "pae/cli/AppConfig.hpp"
#include "pae/factory/Registry.hpp"
#include "pae/io/Errors.hpp"

int main(int argc, char** argv) {
    try {
        pae::factory::registerAll();
        pae::cli::App app{ pae::cli::parseArgs(argc, argv) };
        return app.run();
    } catch (const pae::cli::UsageError& e) {
        std::cerr << "pae: " << e.what() << '\n';
        std::cerr << "Try `pae --help`.\n";
        return 1;
    } catch (const pae::Error& e) {
        std::cerr << "pae: " << e.what() << '\n';
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "pae: unexpected: " << e.what() << '\n';
        return 2;
    }
}
