#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace pae::core {

/// Minimal C++17 polyfill of `std::span<const T>` for our needs.
/// Covers iteration, indexing, size, empty, data. Non-owning view.
///
/// We avoid pulling in C++20's <span> so the project remains C++17.
template <typename T>
class Span {
public:
    using value_type = T;
    using size_type  = std::size_t;
    using iterator   = const T*;

    constexpr Span() noexcept = default;
    constexpr Span(const T* data, size_type size) noexcept : data_(data), size_(size) {}

    template <typename Container,
              typename = std::enable_if_t<
                  !std::is_same_v<std::decay_t<Container>, Span<T>> &&
                  std::is_pointer_v<decltype(std::data(std::declval<const Container&>()))>>>
    Span(const Container& c) noexcept   // NOLINT(google-explicit-constructor)
        : data_(std::data(c)), size_(std::size(c)) {}

    [[nodiscard]] constexpr const T* data()  const noexcept { return data_; }
    [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
    [[nodiscard]] constexpr bool      empty() const noexcept { return size_ == 0; }

    [[nodiscard]] constexpr const T& operator[](size_type i) const noexcept {
        return data_[i];
    }
    [[nodiscard]] constexpr const T& front() const noexcept { return data_[0]; }
    [[nodiscard]] constexpr const T& back()  const noexcept { return data_[size_ - 1]; }

    [[nodiscard]] constexpr iterator begin() const noexcept { return data_; }
    [[nodiscard]] constexpr iterator end()   const noexcept { return data_ + size_; }

private:
    const T*  data_{nullptr};
    size_type size_{0};
};

}  // namespace pae::core
