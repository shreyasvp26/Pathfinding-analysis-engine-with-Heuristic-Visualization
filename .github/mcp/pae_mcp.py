"""
Stub MCP (Model Context Protocol) server for the Pathfinding Analysis Engine.

This file is a placeholder/contract. To make it executable, install the
official `mcp` Python package and flesh out each tool below. The
patterns mirror `../ssm_calender/.github/mcp/ssm_app_mcp.py` and
`../Jyotish AI/.github/mcp/jyotish_ai_mcp.py`.

Tools provided to Copilot:

    run_ctest                   - run the test suite, return summary
    run_clang_tidy              - run clang-tidy on a file/dir
    list_open_bugs              - parse docs/BUGS.md Open section
    list_pending_features       - parse docs/FEATURES.md rows with Pending status
    get_changelog_unreleased    - read the [Unreleased] block
    get_project_version         - read pae/CMakeLists.txt project(... VERSION ...)
    get_git_status, get_git_tags, search_source_code

Run with:

    python .github/mcp/pae_mcp.py
"""

from __future__ import annotations

import re
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
DOCS_DIR  = REPO_ROOT / "docs"
PAE_DIR   = REPO_ROOT / "pae"


def run_ctest(label: str | None = None, build_dir: str = "pae/build") -> dict:
    """Run ctest and return (passed, failed, output)."""
    cmd = ["ctest", "--test-dir", build_dir, "--output-on-failure"]
    if label:
        cmd += ["-L", label]
    proc = subprocess.run(cmd, capture_output=True, text=True, cwd=REPO_ROOT)
    out = proc.stdout + proc.stderr
    m_pass = re.search(r"(\d+) test(?:s)? passed", out)
    m_fail = re.search(r"(\d+) test(?:s)? failed", out)
    return {
        "exit_code": proc.returncode,
        "passed": int(m_pass.group(1)) if m_pass else None,
        "failed": int(m_fail.group(1)) if m_fail else None,
        "output_tail": out[-2000:],
    }


def run_clang_tidy(path: str = "pae/src", build_dir: str = "pae/build") -> dict:
    cmd = [
        "clang-tidy", "-p", build_dir,
        *map(str, REPO_ROOT.glob(f"{path}/**/*.cpp")),
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True, cwd=REPO_ROOT)
    return {
        "exit_code": proc.returncode,
        "stdout_tail": proc.stdout[-4000:],
        "stderr_tail": proc.stderr[-4000:],
    }


def list_open_bugs() -> list[dict]:
    text = (DOCS_DIR / "BUGS.md").read_text(encoding="utf-8")
    section = re.search(r"## Open\s*(.+?)\s*## Fixed", text, re.S)
    if not section:
        return []
    rows = re.findall(r"\|\s*([SBPDA]\d{3,})\s*\|.*?\|\s*(.+?)\s*\|", section.group(1))
    return [{"id": rid, "title": title} for rid, title in rows]


def list_pending_features() -> list[dict]:
    text = (DOCS_DIR / "FEATURES.md").read_text(encoding="utf-8")
    rows = re.findall(
        r"\|\s*(F-\d{3})\s*\|\s*(.+?)\s*\|\s*(@\w+)\s*\|\s*Pending\s*\|",
        text,
    )
    return [{"id": fid, "title": title, "owner": owner} for fid, title, owner in rows]


def get_changelog_unreleased() -> str:
    text = (DOCS_DIR / "CHANGELOG.md").read_text(encoding="utf-8")
    m = re.search(r"## \[Unreleased\]\s*(.+?)\s*## ", text, re.S)
    return (m.group(1).strip() if m else "")


def get_project_version() -> str:
    cm = (PAE_DIR / "CMakeLists.txt").read_text(encoding="utf-8")
    m = re.search(r"project\(\s*\S+\s+VERSION\s+(\d+\.\d+\.\d+)", cm)
    return m.group(1) if m else "unknown"


def get_git_status() -> str:
    return subprocess.check_output(
        ["git", "status", "--short", "--branch"], cwd=REPO_ROOT, text=True
    )


def get_git_tags() -> list[str]:
    out = subprocess.check_output(
        ["git", "tag", "--sort=-v:refname"], cwd=REPO_ROOT, text=True
    )
    return [t for t in out.splitlines() if t.strip()]


def search_source_code(pattern: str, path: str = "pae") -> list[str]:
    proc = subprocess.run(
        ["git", "grep", "-n", pattern, "--", path],
        capture_output=True, text=True, cwd=REPO_ROOT,
    )
    return proc.stdout.splitlines()[:200]


if __name__ == "__main__":
    # Placeholder. The real MCP entry point would register tools with
    # the `mcp` Python SDK. See the SSM/Jyotish reference servers for
    # the wiring pattern.
    print("pae MCP stub. Tools available:",
          "run_ctest, run_clang_tidy, list_open_bugs, list_pending_features,",
          "get_changelog_unreleased, get_project_version,",
          "get_git_status, get_git_tags, search_source_code")
