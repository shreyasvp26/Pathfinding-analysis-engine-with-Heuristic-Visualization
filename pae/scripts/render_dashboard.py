#!/usr/bin/env python3
"""Render a self-contained HTML dashboard from `pae_bench --json` output.

Inputs
------
One or more JSON files emitted by `pae_bench --json`, or a directory
containing them. Filenames are used to derive the map label
(e.g. `maze_50x50_20260429T231500Z.json` -> map = `maze_50x50`).

Output
------
A single self-contained HTML file with Chart.js (CDN-loaded with
SRI). Each map gets its own card with grouped bar charts for:

  * wall-time median (microseconds, lower is better)
  * nodes expanded (lower = more directed search)
  * path cost (must be equal across optimal algorithms)
  * approx peak memory (bytes)

A second section ("cross-map") plots the same metrics across all
maps so you can see how each (algo, heur) configuration scales.

Constraints
-----------
Python 3 stdlib only — no `pip install` ever required. No new
runtime deps for the engine. See the dep policy in
`.github/copilot-instructions.md` §6.

Usage
-----
    pae/scripts/render_dashboard.py \\
        pae/benchmarks/results/*.json \\
        -o pae/benchmarks/results/dashboard.html

    # or point at a directory
    pae/scripts/render_dashboard.py pae/benchmarks/results/
"""

from __future__ import annotations

import argparse
import json
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


# Chart.js v4.4.4 from jsDelivr, with an SRI hash so a tampered CDN
# response is rejected by the browser. If you bump the version, you
# MUST regenerate the SRI hash from the released artefact.
CHARTJS_URL = "https://cdn.jsdelivr.net/npm/chart.js@4.4.4/dist/chart.umd.min.js"
# Real subresource-integrity hash for the URL above — verified
# on 2026-04-29 with:
#   curl -sSf $URL | openssl dgst -sha384 -binary | openssl base64 -A
# To bump Chart.js, replace BOTH the URL and the hash and re-verify.
CHARTJS_SRI = "sha384-NrKB+u6Ts6AtkIhwPixiKTzgSKNblyhlk0Sohlgar9UHUBzai/sgnNNWWd291xqt"


@dataclass
class RunRow:
    map_name: str
    algorithm: str
    heuristic: str
    wall_us_median: int
    expanded: int
    path_cost: int
    approx_peak_bytes: int

    @property
    def label(self) -> str:
        return f"{self.algorithm}+{self.heuristic}" if self.heuristic else self.algorithm


@dataclass
class DashboardData:
    maps: list[str] = field(default_factory=list)
    rows: list[RunRow] = field(default_factory=list)

    def add(self, map_name: str, payload: dict[str, Any]) -> None:
        if map_name not in self.maps:
            self.maps.append(map_name)
        for r in payload.get("runs", []):
            metrics = r.get("metrics", {})
            wall = metrics.get("wall_us", {})
            self.rows.append(RunRow(
                map_name         = map_name,
                algorithm        = r["algorithm"],
                heuristic        = r.get("heuristic", "") or "",
                wall_us_median   = int(wall.get("median", 0)),
                expanded         = int(metrics.get("expanded", 0)),
                path_cost        = int(metrics.get("path_cost", 0)),
                approx_peak_bytes= int(metrics.get("approx_peak_bytes", 0)),
            ))

    def labels_for_map(self, map_name: str) -> list[str]:
        return [r.label for r in self.rows if r.map_name == map_name]

    def metric_for_map(self, map_name: str, attr: str) -> list[int]:
        return [getattr(r, attr) for r in self.rows if r.map_name == map_name]

    def all_labels(self) -> list[str]:
        seen, out = set(), []
        for r in self.rows:
            if r.label not in seen:
                seen.add(r.label)
                out.append(r.label)
        return out


def _strip_timestamp(name: str) -> str:
    """`maze_50x50_20260429T231500Z` -> `maze_50x50`."""
    parts = name.rsplit("_", 1)
    if len(parts) == 2 and parts[1].endswith("Z") and "T" in parts[1]:
        return parts[0]
    return name


def collect_inputs(paths: list[Path]) -> list[Path]:
    files: list[Path] = []
    for p in paths:
        if p.is_dir():
            files.extend(sorted(p.glob("*.json")))
        elif p.is_file() and p.suffix == ".json":
            files.append(p)
        else:
            print(f"render_dashboard: skipping {p} (not a json file)", file=sys.stderr)
    return files


def load(paths: list[Path]) -> DashboardData:
    data = DashboardData()
    for p in paths:
        try:
            payload = json.loads(p.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            print(f"render_dashboard: failed to read {p}: {exc}", file=sys.stderr)
            continue
        map_name = _strip_timestamp(p.stem)
        data.add(map_name, payload)
    return data


def render_html(data: DashboardData) -> str:
    if not data.rows:
        return _empty_html()

    per_map_sections: list[str] = []
    for i, m in enumerate(data.maps):
        labels = data.labels_for_map(m)
        per_map_sections.append(_per_map_section(
            map_name = m,
            chart_id = f"chart_{i}",
            labels   = labels,
            wall     = data.metric_for_map(m, "wall_us_median"),
            expanded = data.metric_for_map(m, "expanded"),
            cost     = data.metric_for_map(m, "path_cost"),
            mem      = data.metric_for_map(m, "approx_peak_bytes"),
        ))

    cross_section = _cross_map_section(data)
    return _shell(
        body = "\n".join(per_map_sections) + cross_section,
        n_maps = len(data.maps),
        n_rows = len(data.rows),
    )


def _per_map_section(*, map_name: str, chart_id: str,
                     labels: list[str], wall: list[int], expanded: list[int],
                     cost: list[int], mem: list[int]) -> str:
    return f"""
<section class="card">
  <h2>{map_name}</h2>
  <div class="chart-grid">
    <div class="chart-cell"><canvas id="{chart_id}_wall"></canvas></div>
    <div class="chart-cell"><canvas id="{chart_id}_exp"></canvas></div>
    <div class="chart-cell"><canvas id="{chart_id}_cost"></canvas></div>
    <div class="chart-cell"><canvas id="{chart_id}_mem"></canvas></div>
  </div>
  <details>
    <summary>raw numbers</summary>
    {_table(labels, wall, expanded, cost, mem)}
  </details>
</section>
<script>
  paeCharts.push({{ ctx: '{chart_id}_wall', label: 'wall-time median (μs)',
    data: {json.dumps(wall)}, labels: {json.dumps(labels)}, color: 'rgba(56,189,248,.7)' }});
  paeCharts.push({{ ctx: '{chart_id}_exp',  label: 'nodes expanded',
    data: {json.dumps(expanded)}, labels: {json.dumps(labels)}, color: 'rgba(167,139,250,.7)' }});
  paeCharts.push({{ ctx: '{chart_id}_cost', label: 'path cost',
    data: {json.dumps(cost)}, labels: {json.dumps(labels)}, color: 'rgba(34,197,94,.7)' }});
  paeCharts.push({{ ctx: '{chart_id}_mem',  label: 'approx peak memory (B)',
    data: {json.dumps(mem)}, labels: {json.dumps(labels)}, color: 'rgba(251,146,60,.7)' }});
</script>
"""


def _table(labels: list[str], wall: list[int], expanded: list[int],
           cost: list[int], mem: list[int]) -> str:
    rows = "\n".join(
        f"<tr><td>{lbl}</td><td>{w}</td><td>{e}</td><td>{c}</td><td>{m}</td></tr>"
        for lbl, w, e, c, m in zip(labels, wall, expanded, cost, mem)
    )
    return f"""
    <table>
      <thead>
        <tr><th>config</th><th>wall_us(med)</th><th>expanded</th>
            <th>path_cost</th><th>memory_B</th></tr>
      </thead>
      <tbody>{rows}</tbody>
    </table>
    """


def _cross_map_section(data: DashboardData) -> str:
    if len(data.maps) <= 1:
        return ""
    labels = data.all_labels()

    def matrix(attr: str) -> list[list[int]]:
        out: list[list[int]] = []
        for m in data.maps:
            row: list[int] = []
            for lbl in labels:
                hits = [getattr(r, attr) for r in data.rows
                        if r.map_name == m and r.label == lbl]
                row.append(int(hits[0]) if hits else 0)
            out.append(row)
        return out

    payload = {
        "labels":   labels,
        "maps":     data.maps,
        "wall":     matrix("wall_us_median"),
        "expanded": matrix("expanded"),
    }
    return f"""
<section class="card">
  <h2>cross-map comparison</h2>
  <p class="muted">Same configuration, different maps. Watch the
  bars grow as the map gets harder.</p>
  <div class="chart-grid">
    <div class="chart-cell"><canvas id="cross_wall"></canvas></div>
    <div class="chart-cell"><canvas id="cross_exp"></canvas></div>
  </div>
</section>
<script>
  paeCross = {json.dumps(payload)};
</script>
"""


def _shell(*, body: str, n_maps: int, n_rows: int) -> str:
    title = "Pathfinding Analysis Engine — benchmark dashboard"
    return f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>{title}</title>
  <style>
    :root {{
      --bg: #0b1020; --fg: #e2e8f0; --muted: #94a3b8;
      --card: #141b30; --border: #1f2a44;
    }}
    @media (prefers-color-scheme: light) {{
      :root {{ --bg:#f8fafc; --fg:#0f172a; --muted:#475569; --card:#fff; --border:#e2e8f0; }}
    }}
    * {{ box-sizing: border-box; }}
    body {{
      margin: 0; padding: 24px;
      background: var(--bg); color: var(--fg);
      font: 14px/1.5 -apple-system, BlinkMacSystemFont, "Segoe UI", system-ui, sans-serif;
    }}
    h1 {{ margin-top: 0; font-size: 22px; }}
    h2 {{ margin: 0 0 12px 0; font-size: 18px; font-weight: 600; }}
    .muted {{ color: var(--muted); }}
    .card {{
      background: var(--card); border: 1px solid var(--border);
      border-radius: 10px; padding: 18px; margin-bottom: 18px;
    }}
    .chart-grid {{
      display: grid; grid-template-columns: repeat(2, minmax(0,1fr));
      gap: 18px;
    }}
    @media (max-width: 720px) {{ .chart-grid {{ grid-template-columns: 1fr; }} }}
    .chart-cell {{ height: 280px; position: relative; }}
    .chart-cell canvas {{ position: absolute; inset: 0; }}
    table {{ width: 100%; border-collapse: collapse; margin-top: 8px; font-size: 13px; }}
    th, td {{ text-align: left; padding: 6px 10px; border-bottom: 1px solid var(--border); }}
    th {{ color: var(--muted); font-weight: 500; }}
    details summary {{ cursor: pointer; color: var(--muted); user-select: none; }}
    .meta {{ color: var(--muted); font-size: 12px; margin-bottom: 18px; }}
    code {{ background: var(--border); padding: 1px 5px; border-radius: 3px; }}
  </style>
</head>
<body>
  <h1>{title}</h1>
  <p class="meta">{n_maps} map(s), {n_rows} run row(s).
     Generated by <code>pae/scripts/render_dashboard.py</code>.
     Source data is the <code>--json</code> output of <code>pae_bench</code>.</p>

  <script>const paeCharts = []; let paeCross = null;</script>
  {body}

  <script src="{CHARTJS_URL}" integrity="{CHARTJS_SRI}" crossorigin="anonymous"></script>
  <script>
    function makeBar(spec) {{
      const ctx = document.getElementById(spec.ctx);
      if (!ctx) return;
      new Chart(ctx, {{
        type: 'bar',
        data: {{ labels: spec.labels,
                datasets: [{{ label: spec.label, data: spec.data, backgroundColor: spec.color }}] }},
        options: {{
          responsive: true, maintainAspectRatio: false,
          scales: {{ y: {{ beginAtZero: true }} }},
          plugins: {{ legend: {{ position: 'bottom' }} }},
        }},
      }});
    }}
    paeCharts.forEach(makeBar);

    if (paeCross) {{
      const palette = ['rgba(56,189,248,.7)','rgba(167,139,250,.7)',
                       'rgba(34,197,94,.7)','rgba(251,146,60,.7)','rgba(244,114,182,.7)'];
      function grouped(canvasId, label, matrix) {{
        const ctx = document.getElementById(canvasId);
        if (!ctx) return;
        new Chart(ctx, {{
          type: 'bar',
          data: {{ labels: paeCross.labels,
                  datasets: paeCross.maps.map((m, i) => ({{
                    label: m, data: matrix[i],
                    backgroundColor: palette[i % palette.length],
                  }})) }},
          options: {{
            responsive: true, maintainAspectRatio: false,
            scales: {{ y: {{ beginAtZero: true, title: {{ display: true, text: label }} }} }},
            plugins: {{ legend: {{ position: 'bottom' }} }},
          }},
        }});
      }}
      grouped('cross_wall', 'wall-time median (μs)', paeCross.wall);
      grouped('cross_exp',  'nodes expanded',        paeCross.expanded);
    }}
  </script>
</body>
</html>
"""


def _empty_html() -> str:
    return _shell(body="<p>No benchmark runs found.</p>", n_maps=0, n_rows=0)


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(
        description="Render a self-contained HTML dashboard from pae_bench JSON output.",
    )
    p.add_argument("inputs", nargs="+", type=Path,
                   help="JSON files (or a directory) emitted by `pae_bench --json`.")
    p.add_argument("-o", "--out", type=Path,
                   default=Path("dashboard.html"),
                   help="Output HTML path (default: ./dashboard.html).")
    args = p.parse_args(argv)

    json_files = collect_inputs(args.inputs)
    if not json_files:
        print("render_dashboard: no JSON inputs found.", file=sys.stderr)
        return 1
    data = load(json_files)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(render_html(data), encoding="utf-8")
    print(f"render_dashboard: wrote {args.out} "
          f"({len(data.maps)} map(s), {len(data.rows)} run row(s))")
    return 0


if __name__ == "__main__":
    sys.exit(main())
