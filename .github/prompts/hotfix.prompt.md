---
mode: 'agent'
description: 'Critical bug → hotfix branch off latest tag → patch release.'
---

Apply a hotfix:

${input:bugId:Critical bug ID — e.g. "B042"}
${input:bugSummary:One-line description}

## Steps

1. Identify the latest release tag: `git describe --tags --abbrev=0`
   (e.g. `v0.2.0`).
2. Create a hotfix branch off that tag:
   ```bash
   git checkout -b hotfix/${bugId}-short-name v0.2.0
   ```
3. Apply the smallest possible fix. Add a regression test.
4. Run `/parallel-checks`.
5. Update `docs/BUGS.md` (move row to `Fixed`).
6. Update `docs/CHANGELOG.md`:
   - Insert a new `[v0.2.1] - YYYY-MM-DD` heading **above**
     `[v0.2.0]`.
   - Single line: `fix(<scope>): ${bugSummary} ([#PR])`.
7. Bump version: `pae/scripts/bump-version.sh 0.2.1` (use the right
   patch version).
8. Commit: `fix(<scope>): ${bugSummary}` then
   `chore(release): v0.2.1`.
9. Tag: `git tag -a v0.2.1 -m "Hotfix: ${bugSummary}"`.
10. Push: `git push origin hotfix/${bugId}-short-name v0.2.1`.
11. Open PR back into `main` to forward-port the fix:
    `git checkout main && git merge --no-ff hotfix/${bugId}-short-name`.

## Reminder

- A hotfix is for **critical** bugs only (crash on default invocation;
  data loss; CVE).
- Do not bundle anything else into a hotfix branch — no refactors, no
  doc cleanups, no "while-I'm-here" fixes.
- Forward-port is mandatory; otherwise the next minor release silently
  re-introduces the bug.
