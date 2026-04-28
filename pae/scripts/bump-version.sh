#!/usr/bin/env bash
# Bump SemVer in pae/CMakeLists.txt and rename [Unreleased] in
# docs/CHANGELOG.md to [vX.Y.Z] - YYYY-MM-DD.
#
# Usage: pae/scripts/bump-version.sh 0.2.0

set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <new-version, e.g. 0.2.0>"
    exit 1
fi

NEW="$1"
if [[ ! "$NEW" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Version must look like X.Y.Z"
    exit 1
fi

cd "$(git rev-parse --show-toplevel)"

# 1. Update pae/CMakeLists.txt project(VERSION ...).
sed -i.bak -E "s/(VERSION )[0-9]+\.[0-9]+\.[0-9]+/\1${NEW}/" pae/CMakeLists.txt
rm -f pae/CMakeLists.txt.bak

# 2. Rename [Unreleased] -> [vNEW] - YYYY-MM-DD and insert a fresh empty
#    [Unreleased] block on top.
TODAY=$(date -u +"%Y-%m-%d")
python3 - <<PY
from pathlib import Path
p = Path("docs/CHANGELOG.md")
text = p.read_text()
new_header = f"## [Unreleased]\n\n### Added\n- _none_\n\n### Changed\n- _none_\n\n### Fixed\n- _none_\n\n## [${NEW}] - ${TODAY}"
text = text.replace("## [Unreleased]", new_header, 1)
p.write_text(text)
PY

# 3. Show what changed.
git diff --stat pae/CMakeLists.txt docs/CHANGELOG.md
echo "Bumped to v${NEW}. Now commit and tag:"
echo "  git add pae/CMakeLists.txt docs/CHANGELOG.md"
echo "  git commit -m \"chore(release): v${NEW}\""
echo "  git tag v${NEW} && git push --tags"
