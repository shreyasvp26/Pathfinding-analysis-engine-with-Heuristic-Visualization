# Pathfinding Analysis Engine — visual artifacts

> Source-controlled Mermaid diagrams. GitHub renders the fences below
> inline; you do not need a local Mermaid CLI to read them. Each diagram
> is also kept as a standalone `.mmd` file so it can be imported into a
> design tool without re-extracting.

## 1. Component view

> File: [`architecture.mmd`](./architecture.mmd) — kept in sync with
> `docs/ARCHITECTURE.md`.

```mermaid
graph TD
    user([CLI user]):::ext
    map[("pae/maps/*.txt")]:::ext

    subgraph pae["pae binary"]
        cli["pae::cli::App<br/>(composition root)"]
        loader["pae::io::GridLoader"]
        registry["pae::factory::Registry&lt;T&gt;<br/>(IPathfinder + IHeuristic)"]
        algo["pae::algo::AStar /<br/>Dijkstra / BFS"]
        heur["pae::heur::Manhattan /<br/>Euclidean / Chebyshev / Octile"]
        viz["pae::viz::CliVisualizer<br/>(or NullVisualizer)"]
        metrics["pae::metrics::Metrics<br/>+ Rss probe"]
        bench["pae::metrics::Benchmark<br/>+ Report"]
    end

    user -- "argv" --> cli
    cli -- "load" --> loader
    loader -- "core::Grid" --> cli
    cli -- "create(name)" --> registry
    registry -- "unique_ptr&lt;...&gt;" --> cli
    cli -- "run(grid, cfg, viz, &m)" --> algo
    algo -- "uses" --> heur
    algo -- "draws via" --> viz
    algo -- "fills" --> metrics
    cli -- "sweep(grid, ...)" --> bench
    bench -- "instantiates" --> algo
    map -- "ASCII" --> loader
    metrics -- "summary" --> user
    bench -- "table / CSV / JSON" --> user

    classDef ext fill:#e8e8e8,stroke:#666,stroke-dasharray:3 3,color:#222;
```

---

## 2. UML class diagram

> File: [`class_diagram.mmd`](./class_diagram.mmd) — mirrors
> `docs/LLD.md` signatures. **If a class signature changes in code,
> change this file in the same PR.**

```mermaid
classDiagram
    direction LR

    class IPathfinder {
        <<interface>>
        +run(grid, cfg, viz*, m*) Result
        +name() string_view
    }
    class AStar {
        -h: IHeuristic&
        +AStar(IHeuristic&)
        +run(...) Result
    }
    class Dijkstra {
        +run(...) Result
    }
    class BFS {
        +run(...) Result
    }

    class IHeuristic {
        <<interface>>
        +estimate(a, b) double
        +name() string_view
    }
    class Manhattan
    class Euclidean
    class Chebyshev
    class Octile

    class IVisualizer {
        <<interface>>
        +onStart(grid) void
        +onExpand(node, grid) void
        +onPath(path, grid) void
        +onFinish(metrics) void
    }
    class CliVisualizer
    class NullVisualizer

    class Grid {
        -cells_: vector~Cell~
        -start_, end_: Coord
        +at(c) Cell
        +width() int
        +height() int
        +inBounds(c) bool
    }

    class Metrics {
        +nodesExpanded: i64
        +nodesEnqueued: i64
        +pathLength:    i64
        +pathCost:      i64
        +wallMicros:    i64
        +approxPeakBytes: i64
        +rssDeltaBytes: i64
    }

    class Result {
        +found: bool
        +path:  vector~Coord~
        +totalCost: i64
    }

    class App {
        -cfg_: AppConfig
        +run() int
    }

    class Registry~T~ {
        +reg(name, factory)
        +create(name) unique_ptr~T~
        +names() vector~string~
    }

    IPathfinder <|.. AStar
    IPathfinder <|.. Dijkstra
    IPathfinder <|.. BFS

    IHeuristic <|.. Manhattan
    IHeuristic <|.. Euclidean
    IHeuristic <|.. Chebyshev
    IHeuristic <|.. Octile

    IVisualizer <|.. CliVisualizer
    IVisualizer <|.. NullVisualizer

    AStar ..> IHeuristic : depends on
    AStar ..> Grid       : reads
    AStar ..> Metrics    : writes
    AStar ..> IVisualizer: notifies

    App ..> Registry : creates
    App ..> IPathfinder : owns
    App ..> IHeuristic  : owns
    App ..> Grid : owns
    App ..> CliVisualizer
    App ..> NullVisualizer

    AStar --> Result
    Dijkstra --> Result
    BFS --> Result
```

---

## 3. Single-run sequence

> File: [`sequence_run.mmd`](./sequence_run.mmd).

```mermaid
sequenceDiagram
    actor User
    participant App as cli::App
    participant Loader as io::GridLoader
    participant Reg as factory::Registry
    participant H as heur::Manhattan
    participant A as algo::AStar
    participant V as viz::CliVisualizer
    participant M as metrics::Metrics

    User->>App: argv
    App->>Loader: loadFromFile(map)
    Loader-->>App: core::Grid
    App->>Reg: create("manhattan")
    Reg-->>App: unique_ptr<IHeuristic>
    App->>A: AStar(*h)
    App->>A: run(grid, cfg, viz, &m)
    A->>V: onStart(grid)
    loop while open set non-empty
        A->>A: pop best, push neighbours
        A->>V: onExpand(node, grid)
    end
    A->>V: onPath(path, grid)
    A->>M: write metrics
    A->>V: onFinish(metrics)
    A-->>App: Result
    App-->>User: summary lines + exit code
```

---

## 4. Benchmark sweep sequence

> File: [`sequence_benchmark.mmd`](./sequence_benchmark.mmd).

```mermaid
sequenceDiagram
    actor User
    participant App as cli::App
    participant Bench as metrics::Benchmark
    participant Reg as factory::Registry
    participant Algo as algo::IPathfinder
    participant Heur as heur::IHeuristic
    participant M as metrics::Metrics
    participant Rep as metrics::Report

    User->>App: argv (+ --benchmark)
    App->>Bench: sweep(grid, algos[], heurs[], cfg)
    loop for each (algo, heur) cell
        loop reps + warmup
            Bench->>Reg: create(algo)
            Reg-->>Bench: unique_ptr<IPathfinder>
            opt astar
                Bench->>Reg: create(heur)
                Reg-->>Bench: unique_ptr<IHeuristic>
            end
            Bench->>Algo: run(grid, cfg, NullViz, &m)
            Algo->>M: fill metrics
            Algo-->>Bench: Result
        end
    end
    Bench-->>App: vector<BenchmarkRun>
    App->>Rep: Report(runs)
    App->>Rep: printTable(stdout)
    Rep-->>User: aggregated table (median + p95)
```
