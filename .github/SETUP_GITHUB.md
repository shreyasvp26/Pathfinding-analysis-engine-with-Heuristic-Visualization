# Push to GitHub & enable Copilot features

> Mirrors `../ssm_calender/.github/SETUP_GITHUB.md`. Adapt commands to
> your shell (zsh on macOS by default; PowerShell on Windows; bash on
> Linux).

## Prerequisites

1. **Git** — pre-installed on macOS/Linux; Windows: <https://git-scm.com/download/win>
2. **GitHub CLI** (recommended for the prompts):
   - macOS: `brew install gh`
   - Windows: `winget install --id GitHub.cli`
   - Linux: see <https://github.com/cli/cli#installation>
3. **CMake ≥ 3.20** — `brew install cmake`, `winget install Kitware.CMake`, or your package manager.
4. **A C++17 compiler.**
   - macOS: `xcode-select --install` (Apple clang).
   - Linux: `sudo apt install build-essential`.
   - Windows: Visual Studio 2022 with the "Desktop development with C++" workload.
5. **clang-format / clang-tidy 14+** (matching CI).

---

## Step 1 — Local clone & first build

```bash
cd ~/Desktop/Projects
git clone https://github.com/<your-org>/pathfinding-analysis-engine.git
cd "Pathfinding Analysis Engine with Heuristic Visualization"
cmake -S . -B pae/build -DCMAKE_BUILD_TYPE=Release
cmake --build pae/build -j
ctest --test-dir pae/build --output-on-failure
```

Expected: 0 failures.

---

## Step 2 — Push the existing repo to GitHub

The repo is already initialised locally with a `main` branch. To push:

```bash
gh repo create pathfinding-analysis-engine \
    --public \
    --source=. \
    --remote=origin \
    --description="Modular C++17 pathfinding engine: A*, Dijkstra, BFS with pluggable heuristics + CLI viz + benchmarks." \
    --push
```

If `gh` is not available, the manual path:

```bash
git remote add origin git@github.com:<your-username>/pathfinding-analysis-engine.git
git branch -M main
git push -u origin main
```

---

## Step 3 — Verify CI runs

1. Open `https://github.com/<you>/pathfinding-analysis-engine/actions`.
2. The **Build & Test** workflow (`ci.yml`) must run on every push to
   `main` or PR. It runs the matrix Linux × macOS × Windows.
3. The **PR Validation** workflow (`validate.yml`) runs lightweight
   checks (clang-format + clang-tidy) on PRs.
4. The **Auto-assign Ticket** workflow (`auto-ticket.yml`) prefixes
   issue titles with the collaborator code.

---

## Step 4 — Configure repo settings

Recommended:

| Setting | Value | Path |
|---------|-------|------|
| Default branch | `main` | Settings → Branches |
| Branch protection (`main`) | Require PR + 1 review + status checks (`ci`, `validate`) | Settings → Branches → Add rule |
| Squash merges only | enabled | Settings → General |
| Allow auto-merge | enabled | Settings → General |
| Dependabot alerts | enabled | Settings → Code security |
| Secret scanning | enabled | Settings → Code security |

Repository topics (Settings → "About"):
`cpp`, `cmake`, `pathfinding`, `astar`, `dijkstra`, `bfs`, `heuristics`,
`visualization`, `oop`, `cli`.

---

## Step 5 — Enable Copilot coding agent (optional)

1. **Settings → Copilot → Coding agent → Enable.**
2. Open an Issue (e.g., *"F-104: implement Manhattan heuristic"*).
3. Assign to **Copilot**.
4. Copilot reads:
   - this repo's `.github/copilot-instructions.md`
   - `AGENTS.md` for the right persona
   - the relevant `.github/instructions/*.instructions.md`
   - and (for big tasks) a matching `.github/prompts/*.prompt.md`.
5. It opens a PR; CI runs automatically; Copilot self-corrects on
   failures.

---

## Step 6 — MCP server (VS Code, optional)

The repo includes a stub MCP server at `.github/mcp/pae_mcp.py`. It
provides Copilot with project-aware tools:

- `run_ctest` — run the test suite, return pass/fail summary.
- `run_clang_tidy` — run clang-tidy on a file or the whole tree.
- `list_open_bugs` — read `docs/BUGS.md`.
- `list_pending_features` — read `docs/FEATURES.md`.
- `get_changelog_unreleased` — read the `[Unreleased]` block.
- `get_project_version` — read the version line from `pae/CMakeLists.txt`.
- `get_git_status`, `get_git_tags`.

Auto-registered via `.vscode/settings.json`. No manual setup needed.

---

## Step 7 — Daily workflow

1. **Start of day**: `git pull`. Check `docs/BUGS.md` Open list. Pick
   the highest-severity bug or the top-of-list `Pending` feature in
   `docs/FEATURES.md`.
2. **Branch**: `git checkout -b feature/F<id>-short-name` (or
   `bugfix/B<id>-short-name`).
3. **Develop**: invoke the matching agent or prompt. Implement.
4. **Verify**: `/parallel-checks` (or run the commands manually — see
   `docs/TESTING.md` §10).
5. **Document**: update `docs/FEATURES.md`, `docs/CHANGELOG.md`,
   `docs/BUGS.md` as relevant.
6. **PR**: open a PR using the template in
   `.github/PULL_REQUEST_TEMPLATE.md`. CI must be green; one review.
7. **Merge**: squash merge. The squash commit message uses
   conventional-commits format (`feat(algo): …`).

---

## Step 8 — Tag a release

```bash
# Update version in pae/CMakeLists.txt and rename [Unreleased] in
# docs/CHANGELOG.md to [vX.Y.Z] - YYYY-MM-DD.
./pae/scripts/bump-version.sh 0.2.0

git add pae/CMakeLists.txt docs/CHANGELOG.md
git commit -m "chore(release): v0.2.0"
git tag v0.2.0
git push --tags
```

The `release.yml` workflow:
- builds Release binaries for Linux x86_64, macOS arm64, Windows x86_64.
- attaches them to the GitHub Release as
  `pae-vX.Y.Z-<os>-<arch>.tar.gz` (or `.zip` on Windows).
- includes a SHA-256 checksum file.

---

## Key file locations

| What | Where |
|------|-------|
| Copilot master rules | `.github/copilot-instructions.md` |
| Per-agent personas | `.github/agents/` (7 files) |
| Auto-applied instructions | `.github/instructions/` (6 files) |
| Reusable prompts | `.github/prompts/` (15+ files) |
| MCP server | `.github/mcp/pae_mcp.py` |
| CI: build + test | `.github/workflows/ci.yml` |
| CI: lint + format | `.github/workflows/validate.yml` |
| CI: auto-ticket | `.github/workflows/auto-ticket.yml` |
| CI: release binaries | `.github/workflows/release.yml` |
| Bug tracker | `docs/BUGS.md` |
| Feature tracker | `docs/FEATURES.md` |
| Changelog | `docs/CHANGELOG.md` |
| Version source of truth | `pae/CMakeLists.txt` |
| Version bump script | `pae/scripts/bump-version.sh` |
