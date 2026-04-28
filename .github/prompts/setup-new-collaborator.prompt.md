---
mode: 'agent'
description: 'Onboard a new contributor to the repo.'
---

Onboard a new contributor:

${input:githubUsername:Their GitHub handle — e.g. "abcuser"}
${input:tickerPrefix:Single uppercase letter for issue auto-prefix — e.g. "A" → A001:, A002:, …}

## Steps

1. **Repo access**
   - Add `${githubUsername}` as a collaborator with `Write` role:
     ```bash
     gh api -X PUT \
       repos/<org>/pathfinding-analysis-engine/collaborators/${githubUsername} \
       -f permission=write
     ```

2. **Auto-ticket prefix**
   - Edit `.github/workflows/auto-ticket.yml`:
     ```yaml
     const prefixMap = {
       'shreyasvp':       'S',
       '${githubUsername}': '${tickerPrefix}',
     };
     ```
   - Commit: `ci(auto-ticket): add ${githubUsername} → ${tickerPrefix}`.

3. **Local setup walkthrough**
   - Send them `.github/SETUP_GITHUB.md` and ask them to follow it
     end-to-end.
   - Sanity check: they run `cmake -S . -B pae/build && cmake --build pae/build && ctest --test-dir pae/build` successfully.

4. **First task — onboarding ramp**
   - Pick a `Pending` row in `docs/FEATURES.md` labelled appropriate
     for their experience (suggest a small `@viz` or `@heuristic`
     task).
   - Assign the issue to them.
   - Ask them to use `/ship-feature` to implement it end-to-end.

5. **Code review etiquette**
   - Tell them: PRs use the template; CI must be green; one approving
     review.
   - Tell them: when in doubt, link the relevant section of `docs/`.
   - Tell them: respect the agent file-ownership rules in `AGENTS.md`.

6. **Communication**
   - Issues are the primary discussion channel.
   - PR comments are for code-specific feedback.
   - Architectural questions go to `Discussions` (if enabled) or a
     dedicated issue tagged `discussion`.
