# Implementation Plan: Baseline Replay Verification

**Branch**: `001-baseline-replay-verification` | **Date**: 2026-04-22 | **Spec**: `specs/001-baseline-replay-verification/spec.md`  
**Input**: Feature specification from `specs/001-baseline-replay-verification/spec.md`

## Summary

Implement and maintain a strict deterministic replay verification baseline for single-player runs using
the Option A compact text artifact model (`*.sidecar`), deterministic ordering and logical-frame cadence,
schema-version parity as a hard gate, and first-divergence diagnostics. Extend the filesystem contract:
canonical binary tapes as **`<basename>.tape`** after normalizing replay tokens from **`-h`/`-v`**;
numbered sidecars **`basename.<n>.sidecar`** with **`max(existing n)+1`** allocation; **`replay-compare`**
supports two-path compares and **single-basename discover mode** (exactly **two** sidecars ⇒ one compare;
**more than two** ⇒ list matches, exit error). Visual playback validation and truncated-tail comparison policy remain in scope per
wspec.

## Technical Context

**Language/Version**: C++ (legacy Sopwith 3 codebase), Windows/MSYS2 SDL maintained path  
**Primary Dependencies**: SDL 1.2, existing replay/history code (`sopwith.cpp` `recordfilename` /
`playbackfilename`), `replay_writer*` / `replay_compare*` toolchain  
**Storage**: Files only — canonical binary **`*.tape`** next to numbered UTF-8 **`*.sidecar`** verification artifacts  
**Testing**: Repeat-run SESSION/full-artifact equality, `replay-compare` two-file and basename-discover modes,
`replay-tests` / `test.bat`, optional `scripts/replay/verify-baseline.ps1` integration  
**Target Platform**: Windows 11 (MSYS2 MinGW64 baseline)  
**Project Type**: Native desktop game + standalone comparator executable  
**Performance Goals**: Comparator remains single-pass for each two-file invocation  
**Constraints**: Preserve gameplay baseline (Principle I); normalization must not fork tape identity;
sidecar numbering must follow **max(n)+1** scan in tape directory  
**Scale/Scope**: Single-player baseline first; multiplayer parity deferred

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- **Principle I — Preserve gameplay baseline**: PASS — file naming changes only affect I/O paths, not sim rules.
- **Principle II — Replay artifacts canonical**: PASS — **`*.tape` / `*.sidecar`** and comparator outputs remain the diagnostic surface.
- **Principle III — Verification gates**: PASS — repeatability, first-divergence, and playback smoke remain required evidence.
- **Principle IV — Contracts**: PASS — logical-frame cadence and ordering unchanged; artifact schema parity enforced before field compare.
- **Principle V — Scope / toolchain**: PASS — Windows MSYS2 path preserved; replay docs must ship with workflow changes.

No justification entries required in Complexity Tracking.

## Project Structure

### Documentation (this feature)

```text
specs/001-baseline-replay-verification/
├── plan.md              # This file (/speckit.plan)
├── research.md          # Phase 0 — resolved policy decisions
├── data-model.md        # Phase 1 — artifact/session/frame/divergence + path naming model
├── quickstart.md        # Phase 1 — maintainer validation steps
├── contracts/
│   └── replay-verification-contract.md
└── tasks.md             # Phase 2 (/speckit.tasks) — not produced by /speckit.plan
```

### Source Code (areas touched)

```text
sopwith3/src/
├── sopwith.cpp           # replay token → normalized .tape opens; sidecar path + allocation
├── replay_compare.cpp         # core two-file compare
├── replay_writer.cpp          # unchanged stream API; path chosen by caller
├── Makefile.msys2 / Makefile  # replay-compare target

sopwith3/tools/
└── replay_compare_tool.cpp    # CLI: two paths + single-basename discover (exactly 2 ⇒ compare)

sopwith3/scripts/replay/
└── verify-baseline.ps1   # LeftArtifact/RightArtifact paths → align with *.sidecar

sopwith3/docs/
├── replay-usage.md
└── findings.md              # operational notes / evidence
```

**Structure Decision**: Single native project under `sopwith3/`; specification artifacts isolated under `specs/001-baseline-replay-verification/`.

## Phase 0 — Research outcome

Captured in `research.md`. Prior comparator/session policies unchanged. Added filesystem and CLI decisions (tape normalization, sidecar allocation, basename discover rules). No unresolved NEEDS CLARIFICATION items remain for implementation planning.

## Phase 1 — Design outcome

| Artifact | Purpose |
|---------|---------|
| `data-model.md` | Artifact rows (`SESSION`, frame blocks, divergence) plus **tape/sidecar path** semantics |
| `contracts/replay-verification-contract.md` | Row/order/compare rules + **file naming and replay-compare** CLI contract |
| `quickstart.md` | Build/run/compare steps aligned with **`*.tape`** / **`*.n.sidecar`** |

Agent context marker in `.cursor/rules/specify-rules.mdc` updated to reference this plan.

## Phase 2 — Implementation planning *(stop here for /speckit.plan)*

Workstreams for `/speckit.tasks` / implementation (not scheduled in this command):

1. **Tape normalization helper** shared by record/playback open paths in `sopwith.cpp`; apply before `fstream::open`; keep `replay_tape_basename`/session identity consistent with stripped basename sans `.tape`.
2. **Sidecar path**: resolve directory of normalized tape file; glob/match **`basename.*.sidecar`**; allocate **`max(n)+1`**; replace **`.state.txt`** construction.
3. **`replay-compare`**: preserve **two-argument** mode; implement **single-argument basename** per FR-016 — discovery, **exactly two** ⇒ **`replay_compare_files`**; **&gt;2** ⇒ print matches, non-zero; **&lt;2** ⇒ non-zero.
4. **Docs & scripts**: `replay-usage.md`, `verify-baseline.ps1`, `.gitignore` patterns for **`*.sidecar`** as needed.
5. **Verification**: unit/integration tests covering normalization edge cases and basename-discover exit codes (**0/1/2+** matches).

## Post-design constitution re-check

| Principle | Result |
|-----------|--------|
| I | PASS |
| II | PASS |
| III | PASS |
| IV | PASS |
| V | PASS |

## Complexity Tracking

None.
