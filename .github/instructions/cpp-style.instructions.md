---
description: 'C++17 style and idioms. Auto-applies to every C++ source file in the repo.'
applyTo: 'pae/**/*.{hpp,cpp,h,cc}'
---

# C++ style — Pathfinding Analysis Engine

These rules are enforced by `.clang-format`, `.clang-tidy`, and CI.
They are repeated here so an agent editing a file knows the rules
without having to read the configs.

## Language level

- **C++17.** No GNU/MSVC extensions.
- Build flags (Linux/macOS): `-Wall -Wextra -Werror -Wpedantic
  -Wshadow -Wconversion -Wsign-conversion -Wold-style-cast`.
- Build flags (Windows MSVC): `/W4 /WX /permissive-`.

## Headers

- `#pragma once` at the top of every header. **No** `#ifndef` guards.
- Include order:
  1. The matching header (`AStar.cpp` includes `pae/algorithms/AStar.hpp` first).
  2. Other `"pae/..."` headers.
  3. C/C++ standard library `<...>` headers.
  4. Third-party headers (rare; Catch2 only in tests).
  Each block separated by a blank line.
- `using namespace` is **forbidden in headers**. In `.cpp`, allowed
  only at function scope (`using std::vector;`).
- Forward-declare in headers when possible. Include only what you
  need.
- No `inline` on member functions defined in the class body — that
  inline is implicit.

## Types

- `int32_t`, `int64_t`, `uint8_t`, etc. via `<cstdint>` for
  fixed-width semantics. Plain `int` is acceptable for small ad-hoc
  counters in a function body.
- `std::size_t` for sizes / loop counters when interfacing with
  containers.
- `double` for any floating-point — never `float` (precision is cheap;
  bugs are not).
- `enum class`, never plain `enum`.
- Underlying type explicit on `enum class` (e.g., `enum class Cell : uint8_t`)
  if the value is packed.

## Naming

| Kind | Convention | Example |
|------|-----------|---------|
| Namespaces | `lower_snake` | `pae::algo` |
| Classes / structs | `CamelCase` | `Grid`, `AStar` |
| Interfaces (pure-abstract) | `I`-prefixed `CamelCase` | `IPathfinder` |
| Methods, functions | `camelCase` | `inBounds`, `estimate` |
| Member variables | `camelCase_` (trailing `_`) | `cells_`, `width_` |
| Constants | `UPPER_SNAKE` | `MAX_GRID_DIM` |
| Enum values | `CamelCase` | `Cell::Empty` |
| File names | `CamelCase.hpp` matches primary type | `AStar.hpp` |
| CMake target | `pae_<module>` (lowercase) | `pae_algorithms` |

## Ownership

- **Owning** a heap object: `std::unique_ptr<T>`. Use
  `std::make_unique`, never raw `new`.
- **Borrowing**: `T&` (preferred) or `const T*`. Never `T*` for owned
  resources in interfaces.
- `std::shared_ptr` only when ownership is genuinely shared. (It is
  not, in v1.)
- `std::move` when transferring ownership, even on `unique_ptr`.

## Memory & performance

- **No allocations in hot loops.** Reserve `std::vector` capacity at
  the start of an algorithm `run()`.
- **No `std::endl`** — always `"\n"`. `std::endl` flushes on every
  call.
- **Avoid `std::map` / `std::set`.** Prefer `std::unordered_map` /
  `std::unordered_set` if you need them at all; usually you don't.
- **Avoid `std::function`** in hot paths (allocates, indirect call);
  use template / function pointer.
- **Avoid `std::shared_ptr`** generally.
- **Prefer `emplace_back` over `push_back`** for non-trivial elements.

## Error handling

- Throw `std::out_of_range`, `std::invalid_argument`, etc. for
  programmer errors.
- Throw the project's own exceptions (`pae::IoError`,
  `pae::UnknownNameError`, `pae::cli::UsageError`) for user-facing
  errors. They all inherit `pae::Error` which inherits
  `std::runtime_error`.
- **Never throw from `noexcept`.** If you mark a method `noexcept` and
  it can throw, you have a bug.
- "Not found" / "no path" is **not an error** — return a result type
  with a `bool found` flag.
- No bare `catch (...)`. (clang-tidy + the pre-commit grep block this.)

## Const-correctness

- Methods that don't mutate are `const`.
- Parameters: `const T&` for read-only, `T&` for output, `T` for
  small (≤ 16 bytes), trivially-copyable values, `T&&` for sink.
- `const` on local variables when their value is computed once and
  never changed.

## Concurrency

- v1 is single-threaded. **No `std::mutex`, no `std::atomic`.**
- If you find yourself wanting either, the design is wrong; ask in the
  PR.

## Banned

- `using namespace std;` in headers (and discouraged in `.cpp`).
- Raw `new` / `delete`.
- C-style casts (`(int)x`); use `static_cast` / `reinterpret_cast`.
- `goto`.
- VLAs.
- `printf` family in production code (tests may use `Catch2` macros).
- `std::vector<bool>` (use `std::vector<uint8_t>`).
- Macros for anything that can be a function or template.

## Encouraged

- `[[nodiscard]]` on functions whose result is meaningful.
- `constexpr` wherever it compiles.
- Structured bindings (`auto [k, v] = pair;`).
- `if constexpr` for compile-time branches.
- `std::optional`, `std::variant` (stdlib value types).
- `std::span<const T>` — but it's C++20; in this project use
  `pae::Span<const T>` from `pae/core/Span.hpp`.
