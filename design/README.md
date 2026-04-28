# `design/` — visual artifacts

Source-controlled architectural diagrams that complement
[`docs/ARCHITECTURE.md`](../docs/ARCHITECTURE.md) and
[`docs/LLD.md`](../docs/LLD.md). All diagrams are written in **Mermaid**
so GitHub renders them inline — no Mermaid CLI is required to read them.

## Files

| File                                          | Purpose                                                                                  |
| --------------------------------------------- | ---------------------------------------------------------------------------------------- |
| [`diagrams.md`](./diagrams.md)                | **Start here.** Single page that inline-renders all diagrams below on GitHub.            |
| [`architecture.mmd`](./architecture.mmd)      | Component view: how the binary wires together. Companion to `docs/ARCHITECTURE.md`.      |
| [`class_diagram.mmd`](./class_diagram.mmd)    | UML class diagram for `IPathfinder`, `IHeuristic`, `IVisualizer`, `Grid`, `App`, etc.    |
| [`sequence_run.mmd`](./sequence_run.mmd)      | Sequence diagram: `pae --map ... --algo astar` → load → algorithm → visualizer → metrics. |
| [`sequence_benchmark.mmd`](./sequence_benchmark.mmd) | Sequence diagram: `pae --map ... --benchmark` → `Benchmark::sweep` → `Report::printTable`. |

## Authoring rules

1. **Source of truth is the `.mmd` file.** `diagrams.md` mirrors them
   so GitHub can render the diagrams in one click; if you edit the
   `.mmd`, paste the new body into `diagrams.md` in the same commit.
2. **Use the same names as code.** A class is `pae::algo::AStar`, not
   "AStar Class". A module boundary is `algo`, `heur`, `viz`, `metrics`.
3. **One concern per diagram.** Cross-link from `docs/` rather than
   bloating a single mega-diagram.
4. **No external assets.** Stay inside Mermaid. If we ever need richer
   notation (C4-PlantUML, draw.io), we add a side-by-side rendered SVG
   and document the source.

## Local rendering (optional)

GitHub renders all of these inline. If you want SVG/PNG locally:

```bash
npm install -g @mermaid-js/mermaid-cli
mmdc -i design/architecture.mmd -o design/architecture.svg
```
