# Low-Level Design (LLD) — Pathfinding Analysis Engine

This is the contract layer between
[`ARCHITECTURE.md`](ARCHITECTURE.md) and the concrete C++ in
`pae/include/`. Every class signature here matches the actual header
file in the source tree. If they diverge, the header is wrong (or this
file is — open a `bugfix/Bxxx-lld-drift` issue).

---

## 1. Notation

- All types live in namespace `pae`. Sub-namespaces: `pae::core`,
  `pae::algo`, `pae::heur`, `pae::viz`, `pae::metrics`,
  `pae::io`, `pae::factory`, `pae::cli`.
- `// owns` annotations indicate `unique_ptr` ownership.
- `// borrows` indicates a non-owning raw pointer or reference; the
  caller guarantees lifetime ≥ callee.
- `// noexcept` on every method that genuinely is. We don't sprinkle
  `noexcept` everywhere — only where the contract is real.
- All interfaces are **abstract** (pure-virtual destructor + at least
  one pure-virtual method). All interface destructors are virtual.

---

## 2. Core domain

### 2.1 `pae::core::Coord`

```cpp
struct Coord {
    int32_t row{};
    int32_t col{};

    constexpr Coord() = default;
    constexpr Coord(int32_t r, int32_t c) : row(r), col(c) {}

    constexpr bool operator==(Coord o) const noexcept { return row == o.row && col == o.col; }
    constexpr bool operator!=(Coord o) const noexcept { return !(*this == o); }
};

struct CoordHash {
    size_t operator()(Coord c) const noexcept {
        // Cantor pairing for non-negative grids; xor-rotate for safety.
        return std::hash<uint64_t>{}(
            (static_cast<uint64_t>(c.row) << 32) ^ static_cast<uint32_t>(c.col));
    }
};
```

**Why a struct, not a class?** It is a value type with public data and
no invariants beyond what its members enforce. Trivially copyable,
movable, and hashable. Used everywhere.

### 2.2 `pae::core::Cell`

```cpp
enum class Cell : uint8_t {
    Empty    = 0,
    Obstacle = 1,
    Start    = 2,
    End      = 3,
};
```

`uint8_t` underlying so the row-major buffer in `Grid` packs tightly.

### 2.3 `pae::core::Node` (algorithm-side, not Grid-side)

```cpp
struct Node {
    int32_t  cellIndex;     // row-major index into Grid
    int64_t  gCost;         // exact cost from start
    double   fCost;         // gCost + heuristic; double because Euclidean
    int32_t  parentCell;    // -1 if none
};

struct NodeFCmp {
    // priority_queue<Node, vector<Node>, NodeFCmp> is a min-heap on f.
    bool operator()(const Node& a, const Node& b) const noexcept {
        if (a.fCost != b.fCost) return a.fCost > b.fCost;
        // Tie-break: prefer larger g (closer to goal under admissible h).
        return a.gCost < b.gCost;
    }
};
```

**Tie-breaking rationale.** With a consistent heuristic, breaking ties
toward larger `g` (i.e., nodes that have travelled further) reduces the
explored area significantly on open maps. Without this, A\* still finds
the optimum but expands more nodes — see `docs/ALGORITHMS.md` §A\*.

### 2.4 `pae::core::Grid`

```cpp
class Grid {
public:
    Grid(int32_t width, int32_t height,
         std::vector<Cell> cells,           // owns
         std::vector<int32_t> weights,      // owns; empty => uniform weight 1
         Coord start, Coord end);

    int32_t width()  const noexcept { return width_; }
    int32_t height() const noexcept { return height_; }
    Coord   start()  const noexcept { return start_; }
    Coord   end()    const noexcept { return end_;   }

    bool inBounds(Coord c) const noexcept;
    Cell at(Coord c) const;                 // throws std::out_of_range
    Cell unchecked(Coord c) const noexcept; // caller asserts inBounds
    int32_t weight(Coord c) const noexcept; // 1 if no weight buffer

    // Returns 4 or 8 neighbour indices (out param avoids alloc on hot path).
    void neighbors4(Coord c, std::array<Coord, 4>& out, int& count) const noexcept;
    void neighbors8(Coord c, std::array<Coord, 8>& out, int& count) const noexcept;

    int32_t toIndex(Coord c) const noexcept { return c.row * width_ + c.col; }
    Coord   toCoord(int32_t idx) const noexcept;

private:
    int32_t              width_;
    int32_t              height_;
    std::vector<Cell>    cells_;
    std::vector<int32_t> weights_;
    Coord                start_;
    Coord                end_;
};
```

**Invariants** (asserted in constructor):
1. `cells_.size() == width * height`.
2. `weights_.empty() || weights_.size() == cells_.size()`.
3. `cells_[toIndex(start)] == Cell::Start`.
4. `cells_[toIndex(end)]   == Cell::End`.
5. `start != end`.

**Thread safety.** Immutable after construction; safe to share across
threads (post-V1).

### 2.5 `pae::io::GridLoader`

```cpp
class GridLoader {
public:
    static Grid loadFromFile(const std::filesystem::path& p);
    static Grid loadFromString(std::string_view s);   // for tests
};

// Errors
struct IoError : std::runtime_error { using std::runtime_error::runtime_error; };
```

**File format.**
```
# any line starting with '#' is a comment
# first non-comment line is "<width> <height>"
# subsequent lines are rows of cells
20 10
####################
#S................##
#.####.....######.E#
...
```
Characters: `.` empty, `#` obstacle, `S` start, `E` end. Optional
weighted variant: a digit `0`–`9` is treated as `Empty` with cell
weight `1+digit`. Validation: exactly one `S`, exactly one `E`,
rectangular, only legal characters.

---

## 3. Heuristics

### 3.1 `pae::heur::IHeuristic`

```cpp
class IHeuristic {
public:
    virtual ~IHeuristic() = default;
    virtual double estimate(Coord from, Coord to) const noexcept = 0;
    virtual std::string_view name() const noexcept = 0;
};
```

**Contract.**
- `estimate(a, a) == 0`.
- `estimate(a, b) == estimate(b, a)` (symmetry).
- `estimate(a, b) <= true_cost(a, b)` (admissibility) for the movement
  model the heuristic was designed for. We document this per heuristic.
- `noexcept` because heuristics are called inside the inner loop; they
  must not throw and must not allocate.

### 3.2 Concrete heuristics

```cpp
class Manhattan : public IHeuristic {
public:
    double estimate(Coord a, Coord b) const noexcept override;
    std::string_view name() const noexcept override { return "manhattan"; }
};

class Euclidean : public IHeuristic {
public:
    double estimate(Coord a, Coord b) const noexcept override;
    std::string_view name() const noexcept override { return "euclidean"; }
};

class Chebyshev : public IHeuristic {
public:
    double estimate(Coord a, Coord b) const noexcept override;
    std::string_view name() const noexcept override { return "chebyshev"; }
};
```

When each is appropriate: see [`HEURISTICS.md`](HEURISTICS.md) §3.

---

## 4. Algorithms

### 4.1 `pae::algo::IPathfinder`

```cpp
struct RunConfig {
    bool   diagonal       = false;
    int    visualizeEvery = 1;     // emit viz event every N expansions
    uint64_t seed         = 0;     // deterministic tie-break randomization (unused if 0)
};

struct Result {
    bool                  found{false};
    std::vector<Coord>    path;
    int64_t               totalCost{0};
};

class IPathfinder {
public:
    virtual ~IPathfinder() = default;
    virtual Result run(const Grid&         grid,
                       const RunConfig&    cfg,
                       IVisualizer*        viz,       // borrows; may be nullptr
                       Metrics*            metrics    // borrows; may be nullptr
                       ) const = 0;
    virtual std::string_view name() const noexcept = 0;
};
```

**Why `const`?** A pathfinder has no mutable state. Each call is
independent. This makes it trivially thread-safe and trivially
re-entrant.

### 4.2 `pae::algo::AStar`

```cpp
class AStar : public IPathfinder {
public:
    explicit AStar(const IHeuristic& h) : h_(h) {}        // borrows
    Result run(const Grid&, const RunConfig&,
               IVisualizer*, Metrics*) const override;
    std::string_view name() const noexcept override { return "astar"; }

private:
    const IHeuristic& h_;
};
```

**Note.** `AStar` takes the heuristic by `const&`. It does not own it.
The caller (`App` / `Benchmark`) owns both objects and outlives the
search. This is OOP composition: `AStar` *uses* an `IHeuristic`, it
does not *own* one.

### 4.3 `pae::algo::Dijkstra`

```cpp
class Dijkstra : public IPathfinder {
public:
    Result run(const Grid&, const RunConfig&,
               IVisualizer*, Metrics*) const override;
    std::string_view name() const noexcept override { return "dijkstra"; }
};
```

Dijkstra is "A\* with a zero heuristic." We could implement it that
way, but we keep it as a separate class so:
1. The benchmark output names are honest (`dijkstra` not
   `astar+zero`).
2. We can micro-optimise (e.g., skip the `+ h` arithmetic in the
   inner loop).
3. The OOP demonstration is clearer for readers.

### 4.4 `pae::algo::BFS`

```cpp
class BFS : public IPathfinder {
public:
    Result run(const Grid&, const RunConfig&,
               IVisualizer*, Metrics*) const override;
    std::string_view name() const noexcept override { return "bfs"; }
};
```

BFS uses a `std::queue<int32_t>` (FIFO) instead of a priority queue.
It treats every move as cost 1 — so `Result::totalCost == path.size() - 1`.

---

## 5. Visualization

### 5.1 `pae::viz::IVisualizer`

```cpp
class IVisualizer {
public:
    virtual ~IVisualizer() = default;

    virtual void onSearchStart(const Grid& g)            = 0;
    virtual void onEnqueue(Coord c, double f)            = 0;
    virtual void onExpand(Coord c)                       = 0;
    virtual void onPathFound(std::span<const Coord> p)   = 0;
    virtual void onSearchComplete(const Metrics& m)      = 0;
};
```

**Why event-style?** The algorithm doesn't render. It says "I just
expanded this node" and the visualizer decides whether/how to display
it. `step` mode redraws after each `onExpand`; `final` mode redraws
only on `onPathFound`; `none` mode (`NullVisualizer`) ignores everything
and exists so the algorithms always have a non-null pointer to call —
no `if (viz != nullptr)` branches in the hot loop.

`std::span` requires C++20; we ship a minimal `pae::Span<const T>`
header for C++17 compatibility (see `pae/include/pae/core/Span.hpp`).

### 5.2 `pae::viz::CliVisualizer`

```cpp
enum class VizMode : uint8_t { None, Final, Step };

struct CliConfig {
    VizMode mode      = VizMode::Step;
    bool    color     = true;
    int     fpsCap    = 30;        // throttle redraws in step mode
    bool    clearTerm = true;
};

class CliVisualizer : public IVisualizer {
public:
    explicit CliVisualizer(CliConfig cfg);
    // ... overrides ...
private:
    CliConfig                  cfg_;
    const Grid*                grid_{nullptr};   // borrows; lifetime: between onSearchStart/onSearchComplete
    std::vector<bool>          openSet_;
    std::vector<bool>          closedSet_;
    std::chrono::steady_clock::time_point lastFrame_;
};
```

### 5.3 `pae::viz::NullVisualizer`

```cpp
class NullVisualizer final : public IVisualizer {
public:
    void onSearchStart(const Grid&)              override {}
    void onEnqueue(Coord, double)                override {}
    void onExpand(Coord)                         override {}
    void onPathFound(std::span<const Coord>)     override {}
    void onSearchComplete(const Metrics&)        override {}
};
```

---

## 6. Metrics & benchmarking

### 6.1 `pae::metrics::Metrics`

```cpp
struct Metrics {
    int64_t nodesExpanded   = 0;
    int64_t nodesEnqueued   = 0;
    int64_t pathLength      = 0;     // cells in path
    int64_t pathCost        = 0;     // sum of weights along path
    int64_t wallMicros      = 0;
    int64_t approxPeakBytes = 0;     // open + closed + parent buffers

    void reset() noexcept;
};
```

### 6.2 `pae::metrics::Benchmark`

```cpp
struct BenchmarkRun {
    std::string algoName;
    std::string heuristicName;   // "" for non-A* algorithms
    Metrics     metrics;
};

class Benchmark {
public:
    struct Config {
        int repetitions     = 30;
        bool warmup         = true;
        bool useNullViz     = true;     // remove rendering cost from timing
    };

    static std::vector<BenchmarkRun> sweep(
        const Grid&                              grid,
        std::span<const std::string>             algoNames,       // "astar", "dijkstra", "bfs"
        std::span<const std::string>             heuristicNames,  // "manhattan", "euclidean"
        Config                                   cfg);
};
```

### 6.3 `pae::metrics::Report`

```cpp
class Report {
public:
    explicit Report(std::vector<BenchmarkRun> runs);

    void printTable(std::ostream& os) const;     // pretty CLI table
    void writeCsv(std::ostream& os)   const;
    void writeJson(std::ostream& os)  const;

private:
    std::vector<BenchmarkRun> runs_;
};
```

---

## 7. Factory / registry

### 7.1 `pae::factory::Registry<T>`

```cpp
template <typename T>
class Registry {
public:
    using Factory = std::function<std::unique_ptr<T>()>;

    static Registry& instance();
    void reg(std::string_view name, Factory f);
    std::unique_ptr<T> create(std::string_view name) const;
    std::vector<std::string> names() const;

private:
    std::unordered_map<std::string, Factory> map_;
};
```

### 7.2 `registerAll()`

```cpp
namespace pae::factory {
void registerAll();   // registers built-in algorithms + heuristics
}
```

Lives in `pae/src/factory/register_default.cpp` and pulls in concrete
classes so they don't pollute the lower libraries with header
dependencies.

---

## 8. CLI / app layer

### 8.1 `pae::cli::AppConfig`

```cpp
struct AppConfig {
    std::filesystem::path   mapPath;
    std::string             algo      = "astar";
    std::string             heuristic = "manhattan";
    VizMode                 vizMode   = VizMode::Step;
    bool                    diagonal  = false;
    bool                    color     = true;
    bool                    benchmark = false;
    uint64_t                seed      = 0;
};

AppConfig parseArgs(int argc, char** argv);   // throws pae::cli::UsageError
```

### 8.2 `pae::App`

```cpp
class App {
public:
    explicit App(AppConfig cfg);
    int run();        // returns process exit code
private:
    AppConfig cfg_;
};
```

`main.cpp` is then:

```cpp
int main(int argc, char** argv) {
    try {
        pae::factory::registerAll();
        return pae::App{ pae::cli::parseArgs(argc, argv) }.run();
    } catch (const pae::Error& e) {
        std::cerr << "pae: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "pae: unexpected: " << e.what() << "\n";
        return 2;
    }
}
```

---

## 9. Class relationships (UML-style)

```
                       <<interface>>
                        IPathfinder
                            ▲
               ┌────────────┼─────────────┐
               │            │             │
            AStar       Dijkstra         BFS
               │ uses
               ▼
                       <<interface>>
                        IHeuristic
                            ▲
               ┌────────────┼─────────────┐
               │            │             │
          Manhattan     Euclidean      Chebyshev


  Grid (composes Cells, weights, start/end)         IPathfinder uses Grid (const&)
  Grid is loaded by GridLoader                      IPathfinder writes to Metrics (borrows)
                                                    IPathfinder emits to IVisualizer (borrows)

                       <<interface>>
                        IVisualizer
                            ▲
               ┌────────────┴────────────┐
               │                          │
        CliVisualizer              NullVisualizer
```

Composition (filled diamonds in UML): `Grid` *contains* `Cell`s; `App`
*contains* `AppConfig`. Aggregation (open diamonds): `AStar`
*references* `IHeuristic`; `IPathfinder::run` *references*
`IVisualizer` and `Metrics`. Inheritance (open triangle): every
abstract → concrete arrow above.

---

## 10. OOP principles, mapped to this design

| Principle | Where in this design |
|-----------|---------------------|
| **Abstraction** | `IPathfinder`, `IHeuristic`, `IVisualizer`. Callers depend on the contract, not the implementation. |
| **Encapsulation** | `Grid`'s buffer is private; access only via `at`/`unchecked`/`weight`. `CliVisualizer` hides ANSI escape goo. |
| **Inheritance** | `AStar : IPathfinder`, `Manhattan : IHeuristic`, `CliVisualizer : IVisualizer`. Pure-interface inheritance — no protected state, no diamond hierarchies. |
| **Polymorphism** | `App::run` holds `unique_ptr<IPathfinder>` and calls `run` polymorphically. `AStar` calls `h_.estimate(…)` polymorphically on any `IHeuristic`. |
| **Separation of concerns** | One responsibility per library: core, io, heuristics, algorithms, viz, metrics, factory, app. No library crosses concerns. |
| **Open/Closed** | New algorithm = new `IPathfinder` subclass + one registration line. **No existing class is modified.** |
| **Liskov substitution** | Any `IHeuristic` can be passed to `AStar`. Any `IPathfinder` can be selected by the CLI. The contracts are honoured by every concrete subclass. |
| **Dependency inversion** | High-level `App` depends on `IPathfinder` (abstraction), not on `AStar` (concretion). Composition root in `register_default.cpp`. |

---

## 11. Performance considerations baked into the LLD

1. `Grid::neighbors{4,8}` writes into a stack-allocated `std::array`
   to avoid per-call heap allocation.
2. `Node` is 24 bytes (4+8+8+4) — fits comfortably in cache lines
   when `priority_queue`'s underlying `vector<Node>` is hot.
3. `gScore` and `parent` are `int32_t` indexed (not `Coord`) to halve
   the working set vs. a `unordered_map<Coord, …>`.
4. `closedSet` is implicit (`gScore[idx] != INT64_MAX`) — saves a hash
   set entirely on the hot path.
5. Visualization throttling (`fpsCap`) keeps step mode usable on large
   grids without changing the algorithm.

See [`PERFORMANCE.md`](PERFORMANCE.md) for the full perf model and
where the budget is spent.
