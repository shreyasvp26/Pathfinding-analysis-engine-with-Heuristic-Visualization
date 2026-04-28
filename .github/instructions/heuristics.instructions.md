---
description: 'Auto-applies when editing heuristic functions.'
applyTo: 'pae/**/heuristics/**'
---

# Heuristics — coding rules

## Source of truth

- Class shapes: `docs/LLD.md` §3.
- Behaviour, admissibility proofs, when to use: `docs/HEURISTICS.md`.
- Property tests: `docs/TESTING.md` §4.1.

## The contract (every heuristic must satisfy)

```cpp
class IHeuristic {
public:
    virtual ~IHeuristic() = default;
    virtual double estimate(Coord from, Coord to) const noexcept = 0;
    virtual std::string_view name() const noexcept = 0;
};
```

Required properties (proven, not just tested):

1. **Non-negative.** `estimate(a, b) >= 0.0`.
2. **Goal-zero.** `estimate(a, a) == 0.0`. Watch out for `-0.0`;
   normalise to `+0.0`.
3. **Symmetric.** `estimate(a, b) == estimate(b, a)`.
4. **Admissible** for the documented movement model:
   `estimate(a, b) <= true_cost(a, b)`.
5. **Consistent** (monotone): `estimate(a, b) <= edge_cost(a, c) + estimate(c, b)`.

## Implementation rules

1. **`estimate` is `noexcept`.** No allocation. No I/O. No
   exceptions. It is in A\*'s inner loop.
2. **`estimate` is `const`.** No state. No caching.
3. **`double`, not `float`.**
4. **`std::abs` from `<cstdlib>` or `<cmath>`** for integer arithmetic.
5. **No virtual dispatch inside `estimate`.** It is the virtual.
6. **No use of `Grid`.** A heuristic only sees two `Coord`s; it does
   not know about obstacles or weights.

## Header template

```cpp
#pragma once

#include "pae/heuristics/IHeuristic.hpp"

namespace pae::heur {

/// <Name> distance heuristic.
///
/// Admissible for: <list movement models>.
/// Cost per call: ~N ns on a modern x86-64 box.
class <Name> : public IHeuristic {
public:
    double estimate(core::Coord from, core::Coord to) const noexcept override;
    std::string_view name() const noexcept override { return "<lowercase-name>"; }
};

}  // namespace pae::heur
```

## Implementation template

```cpp
#include "pae/heuristics/<Name>.hpp"

#include <cmath>
#include <cstdlib>

namespace pae::heur {

double <Name>::estimate(core::Coord a, core::Coord b) const noexcept {
    const auto dx = std::abs(a.col - b.col);
    const auto dy = std::abs(a.row - b.row);
    return /* the formula */;
}

}  // namespace pae::heur
```

## Property-test template

Every new heuristic ships with this Catch2 file:

```cpp
TEST_CASE("<Name> is symmetric, non-negative, zero on diagonal",
          "[heuristic][<name>][property]") {
    <Name> h;
    auto r1 = GENERATE(range(0, 32));
    auto c1 = GENERATE(range(0, 32));
    auto r2 = GENERATE(range(0, 32));
    auto c2 = GENERATE(range(0, 32));
    Coord a{r1, c1}, b{r2, c2};
    REQUIRE(h.estimate(a, b) >= 0.0);
    REQUIRE(h.estimate(a, b) == h.estimate(b, a));
    REQUIRE(h.estimate(a, a) == 0.0);
}

TEST_CASE("<Name> is admissible against true cost (sampled)", "[heuristic][<name>]") {
    // Compare h(a, b) <= true_cost(a, b) for the heuristic's movement model.
}
```

## Anti-patterns

| Anti-pattern | Why it's wrong |
|--------------|---------------|
| `int estimate(...)` instead of `double` | Loses precision for Euclidean. |
| Caching results in a `mutable std::map` | Subtle thread-safety bugs once benchmarks parallelise; pointless at our cost-per-call. |
| Calling `Grid::weight` from `estimate` | The heuristic is grid-agnostic; let A\* combine `g + h`. |
| Returning negative for `goal == start` due to `-0.0` | Normalise: `return std::abs(value);` or compute with `>=` from the start. |
