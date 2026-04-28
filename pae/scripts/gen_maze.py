#!/usr/bin/env python3
"""
gen_maze.py — Reproducible map generator for the Pathfinding Analysis Engine.

This is *not* meant to be a pretty maze generator. It is meant to give us
deterministic, parameterised maps so benchmark numbers are comparable
across machines and CI runs.

Usage:
    python3 pae/scripts/gen_maze.py maze   --w 50 --h 50 --seed 42 \\
        --out pae/maps/maze_50x50.txt
    python3 pae/scripts/gen_maze.py arena  --w 50 --h 50 --density 0.20 --seed 7 \\
        --out pae/maps/open_arena_50x50.txt

Modes
-----
maze    Recursive-backtracker maze on a grid where odd indices are walls and
        even indices are passages. Guaranteed connected, exactly one path
        between any two cells (treat as an upper bound on hardness).

arena   Open arena with random obstacles at the given density. Start in the
        top-left, end in the bottom-right. Re-rolls obstacles on the start /
        end cells. Does NOT guarantee connectivity (callers should pick
        densities below ~0.30 in practice).

Both modes always write a header line of the form `<width> <height>` and a
short comment line on top so the output is valid pae/maps/*.txt format.
"""

from __future__ import annotations

import argparse
import random
import sys
from pathlib import Path
from typing import List


def render_maze(width: int, height: int, seed: int) -> str:
    """Recursive backtracker. width/height must be odd >= 5."""
    if width < 5 or height < 5:
        raise ValueError("maze: width/height must be >= 5")
    if width % 2 == 0 or height % 2 == 0:
        raise ValueError("maze: width/height must be odd")

    rng = random.Random(seed)
    grid: List[List[str]] = [["#" for _ in range(width)] for _ in range(height)]

    stack = [(1, 1)]
    grid[1][1] = "."
    while stack:
        cy, cx = stack[-1]
        nbrs = []
        for dy, dx in ((-2, 0), (2, 0), (0, -2), (0, 2)):
            ny, nx = cy + dy, cx + dx
            if 0 < ny < height - 1 and 0 < nx < width - 1 and grid[ny][nx] == "#":
                nbrs.append((ny, nx, dy, dx))
        if not nbrs:
            stack.pop()
            continue
        ny, nx, dy, dx = rng.choice(nbrs)
        grid[cy + dy // 2][cx + dx // 2] = "."
        grid[ny][nx] = "."
        stack.append((ny, nx))

    grid[1][1] = "S"
    grid[height - 2][width - 2] = "E"

    lines = [
        f"# {width}x{height} recursive-backtracker maze, seed={seed}.",
        f"{width} {height}",
    ]
    for row in grid:
        lines.append("".join(row))
    return "\n".join(lines) + "\n"


def render_arena(width: int, height: int, density: float, seed: int) -> str:
    if not (0.0 <= density <= 0.95):
        raise ValueError("arena: density must be in [0, 0.95]")
    rng = random.Random(seed)

    grid = [["." for _ in range(width)] for _ in range(height)]
    for y in range(height):
        for x in range(width):
            if rng.random() < density:
                grid[y][x] = "#"

    grid[0][0] = "S"
    grid[height - 1][width - 1] = "E"

    lines = [
        f"# {width}x{height} open arena, density={density:.2f}, seed={seed}.",
        f"{width} {height}",
    ]
    for row in grid:
        lines.append("".join(row))
    return "\n".join(lines) + "\n"


def main(argv: List[str]) -> int:
    p = argparse.ArgumentParser(description="Generate a pae/maps/*.txt map.")
    sub = p.add_subparsers(dest="mode", required=True)

    pm = sub.add_parser("maze", help="Recursive-backtracker maze.")
    pm.add_argument("--w", type=int, default=51, help="Width  (odd >= 5).")
    pm.add_argument("--h", type=int, default=51, help="Height (odd >= 5).")
    pm.add_argument("--seed", type=int, default=42)
    pm.add_argument("--out", required=True, type=Path)

    pa = sub.add_parser("arena", help="Random-obstacle arena.")
    pa.add_argument("--w", type=int, default=50)
    pa.add_argument("--h", type=int, default=50)
    pa.add_argument("--density", type=float, default=0.20)
    pa.add_argument("--seed", type=int, default=7)
    pa.add_argument("--out", required=True, type=Path)

    args = p.parse_args(argv)

    if args.mode == "maze":
        body = render_maze(args.w, args.h, args.seed)
    else:
        body = render_arena(args.w, args.h, args.density, args.seed)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(body)
    print(f"wrote {args.out} ({args.w}x{args.h}, mode={args.mode})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
