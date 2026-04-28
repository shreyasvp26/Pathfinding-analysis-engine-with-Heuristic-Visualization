# Security Policy — Pathfinding Analysis Engine

This is an offline, single-binary CLI. The attack surface is small but
not zero. This document lists the threats we **do** care about and the
ones we explicitly accept.

---

## Threat model

| Asset | Threat | Mitigation |
|-------|--------|------------|
| Engine binary | Memory-safety bugs (UAF, OOB, buffer overflow) | Strict `-Wall -Wextra -Werror -Wpedantic`; ASan + UBSan in CI; `gsl::span`-style bounds-checked accessors via `Grid::at()` not `operator[]`. |
| Engine binary | Integer overflow on path costs / metrics | Costs stored as `std::int64_t`; explicit overflow check in benchmark accumulator. |
| Map files (`.txt`, `.grid`) | Malicious or malformed input causing crash / DoS | `GridLoader` validates dimensions, character set, single start/end; loads into a fixed-size buffer; rejects files > 64 MB by default. |
| CI / supply chain | Compromised dependency | Only one external dependency: Catch2 (pinned by tag in `FetchContent`). Dependabot updates weekly via [`.github/dependabot.yml`](.github/dependabot.yml). |
| Build system | Arbitrary code execution at configure time | No `execute_process` running fetched scripts. CMake + FetchContent only. |
| Reproducibility | Non-determinism altering benchmark results | All RNG seeded; tie-breaking in priority queues is deterministic; no time-dependent code paths in algorithms. |

---

## Out of scope

| Item | Reason |
|------|--------|
| Network attacks | The engine has **no networking layer**. It does not open sockets, does not download anything at runtime. |
| Auth / authz | The engine has no users, no sessions, no roles. |
| Cryptography | No crypto primitives are used or required. |
| Multi-tenant isolation | Single-process, single-user CLI. |

---

## Memory-safety practices

1. **No raw `new` / `delete`** in production code. Use `std::make_unique`
   or stack allocation. Containers own their data.
2. **No raw owning pointers** in interfaces. Pass `const T&` or
   `std::span<const T>` (or our hand-rolled `pae::Span` for C++17).
3. **All vectors used as buffers** must be `reserve()`'d to expected
   capacity to avoid mid-algorithm reallocation.
4. **`Grid::at(coord)`** does bounds checking and throws
   `std::out_of_range`; algorithms call `Grid::inBounds()` first and
   then use `unchecked()` only after that gate.
5. **Iterators stay valid:** algorithms never mutate the open-set
   container while iterating it; mutations happen through `push` /
   `pop`.
6. **Dangerous functions banned:** `gets`, `strcpy`, `sprintf`,
   `atoi` — enforced by clang-tidy `bugprone-*`,
   `cppcoreguidelines-*`.

---

## Reporting a vulnerability

Open a private GitHub Security Advisory on the repository, or email
the maintainer (see project README). Do **not** open a public issue
for security bugs.

Expected response time: **3 business days** to triage, **14 days** to
patch for confirmed vulnerabilities.

---

## CI security

| Check | Where | Failure = block merge |
|-------|-------|----------------------|
| Compiler warnings as errors | `pae/CMakeLists.txt` (`-Werror`) | yes |
| AddressSanitizer | `.github/workflows/ci.yml` (Debug job) | yes |
| UndefinedBehaviorSanitizer | `.github/workflows/ci.yml` (Debug job) | yes |
| clang-tidy | `.github/workflows/validate.yml` | yes |
| Dependabot — Catch2 minor / patch | `.github/dependabot.yml` | (auto-PR) |
| Pre-commit `clang-format` | `.pre-commit-config.yaml` | local + CI |

---

## Secrets

There are no runtime secrets. The repo must never contain:

- Personal API keys (any service)
- Private SSH keys, certificates, or keystores
- Real user data of any kind

`.gitignore` excludes `.env*`, `*.pem`, `*.key`, `secrets/**`. CI runs
`git diff --check` style scans on PRs.

---

## Supply chain pinning

| Dependency | Version pin | Source |
|------------|------------|--------|
| Catch2 | tag `v3.5.4` (or later semver-compatible) | `https://github.com/catchorg/Catch2.git` via `FetchContent` |
| Google Benchmark (optional) | tag `v1.8.3` | `https://github.com/google/benchmark.git` via `FetchContent`, only if `-DPAE_USE_GBENCH=ON` |

Both are MIT/BSL-licensed. License audit: see
[`docs/EXTENSIONS.md`](docs/EXTENSIONS.md) §Licensing.
