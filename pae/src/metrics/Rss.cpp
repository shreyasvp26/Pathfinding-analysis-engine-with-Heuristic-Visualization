#include "pae/metrics/Rss.hpp"

#if defined(PAE_TRUE_RSS) && (defined(__linux__) || defined(__APPLE__))
    #include <sys/resource.h>
#endif

namespace pae::metrics {

std::int64_t maxResidentBytes() noexcept {
#if defined(PAE_TRUE_RSS) && (defined(__linux__) || defined(__APPLE__))
    rusage ru{};
    if (getrusage(RUSAGE_SELF, &ru) != 0) {
        return 0;
    }
    // ru_maxrss is in KiB on Linux, in bytes on macOS.
    #if defined(__APPLE__)
        return static_cast<std::int64_t>(ru.ru_maxrss);
    #else
        return static_cast<std::int64_t>(ru.ru_maxrss) * 1024;
    #endif
#else
    return 0;
#endif
}

}  // namespace pae::metrics
