---
mode: 'agent'
description: 'Bump version, update CHANGELOG, tag, push. Triggers the release workflow.'
---

Cut a release:

${input:newVersion:New SemVer — e.g. "0.2.0"}
${input:summary:One-paragraph release summary}

## Pre-checks

1. `git status` — working tree clean.
2. `git pull --ff-only` — up to date with `main`.
3. `/parallel-checks` — all green.
4. `docs/CHANGELOG.md` `[Unreleased]` is non-empty (otherwise: nothing
   to release).
5. `docs/BUGS.md` has no `critical` Open bugs.

## Sequence

1. Run `pae/scripts/bump-version.sh ${newVersion}`. This:
   - Updates `pae/CMakeLists.txt` `project(... VERSION x.y.z ...)`.
   - Renames `[Unreleased]` heading in `docs/CHANGELOG.md` to
     `[${newVersion}] - YYYY-MM-DD` (today).
   - Inserts a fresh empty `[Unreleased]` block above it.
2. Verify the diff:
   ```bash
   git diff pae/CMakeLists.txt docs/CHANGELOG.md
   ```
3. Commit:
   ```bash
   git add pae/CMakeLists.txt docs/CHANGELOG.md
   git commit -m "chore(release): v${newVersion}"
   ```
4. Tag:
   ```bash
   git tag -a v${newVersion} -m "v${newVersion}: ${summary}"
   ```
5. Push:
   ```bash
   git push origin main
   git push origin v${newVersion}
   ```
6. The `release.yml` workflow builds Linux/macOS/Windows binaries and
   attaches them to the GitHub Release.
7. Confirm at `https://github.com/<org>/pathfinding-analysis-engine/releases/tag/v${newVersion}`
   that the assets are uploaded:
   - `pae-v${newVersion}-linux-x86_64.tar.gz`
   - `pae-v${newVersion}-macos-arm64.tar.gz`
   - `pae-v${newVersion}-windows-x86_64.zip`
   - `SHA256SUMS.txt`

## Post

1. Open a follow-up issue if the release uncovered any rough edges
   (e.g., release notes that need clarification).
2. Update `docs/ROADMAP.md` if the next planned tag changed.
