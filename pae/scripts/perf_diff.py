#!/usr/bin/env python3
"""
Compare two benchmark JSON outputs from pae_bench --json.

Usage:
    pae/scripts/perf_diff.py before.json after.json
"""
from __future__ import annotations

import json
import sys
from pathlib import Path


def load(p: str) -> dict:
    return json.loads(Path(p).read_text(encoding="utf-8"))


def index(report: dict) -> dict:
    by_key = {}
    for r in report.get("runs", []):
        key = (r["algorithm"], r.get("heuristic", ""))
        by_key[key] = r["metrics"]
    return by_key


def fmt_pct(before: float, after: float) -> str:
    if before == 0:
        return "n/a"
    return f"{(after - before) / before * 100:+.1f}%"


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print(__doc__)
        return 2
    a = index(load(argv[1]))
    b = index(load(argv[2]))
    keys = sorted(set(a) | set(b))
    print(f"{'algo':<10} {'heur':<10} "
          f"{'wall_us(med)':>20} {'expanded':>14} "
          f"{'mem_B':>14}")
    for k in keys:
        ma = a.get(k)
        mb = b.get(k)
        if not ma or not mb:
            continue
        before_us = ma["wall_us"]["median"]
        after_us  = mb["wall_us"]["median"]
        before_e  = ma["expanded"]
        after_e   = mb["expanded"]
        before_m  = ma["approx_peak_bytes"]
        after_m   = mb["approx_peak_bytes"]
        print(f"{k[0]:<10} {k[1] or '-':<10} "
              f"{before_us}->{after_us} ({fmt_pct(before_us, after_us)})  "
              f"{before_e}->{after_e}  "
              f"{before_m}->{after_m}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
