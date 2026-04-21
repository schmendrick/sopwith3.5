# Implementation Plan: Baseline Replay Verification

**Branch**: `001-baseline-replay-verification` | **Date**: 2026-04-21 | **Spec**: `specs/001-baseline-replay-verification/spec.md`
**Input**: Feature specification from `specs/001-baseline-replay-verification/spec.md`

## Summary

Define and implement a strict deterministic replay verification baseline for single-player runs using
the Option A text artifact model, deterministic ordering/cadence contracts, strict schema-version
match enforcement, and first-divergence reporting. Include visual replay playback validation in
feature acceptance and support truncated-tail handling by comparing only complete frame blocks with
warning emission.

## Technical Context

**Language/Version**: C++ (legacy Sopwith 3 codebase, current maintained Windows/MSYS2 SDL path)  
**Primary Dependencies**: SDL 1.2 runtime path, existing replay/history game code, file I/O utilities  
**Storage**: File-based replay artifacts (`.txt` canonical output)  
**Testing**: Deterministic replay comparison checks + repeat-run artifact equality checks + playback smoke checks  
**Target Platform**: Windows 11 (MSYS2 MinGW64 maintained baseline)  
**Project Type**: Native desktop game runtime with deterministic verification tooling  
**Performance Goals**: First-divergence report produced in one comparator pass; replay artifacts remain usable for 3-5 minute single-player sessions  
**Constraints**: Preserve existing gameplay baseline; logical-frame cadence only; strict row ordering; schema mismatch hard fail; missing required row kind hard fail  
**Scale/Scope**: Single-player baseline first; multiplayer/network parity deferred

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- **Principle I - Preserve Gameplay Baseline While Hardening Determinism**: PASS  
  Plan remains single-player-first and avoids gameplay rewrites.
- **Principle II - Replay Artifacts and First-Divergence Diagnostics Are Canonical**: PASS  
  Feature centers on canonical artifact schema and comparator outputs.
- **Principle III - Verification Gates and Reproducible Evidence**: PASS  
  Plan includes repeatability checks, mismatch checks, and playback validation evidence.
- **Principle IV - Stable Identity, Ordering, and Logical-Frame Contracts**: PASS  
  Ordering and cadence contracts are explicit and enforced in design artifacts.
- **Principle V - Scope Discipline, Toolchain Reality, and Legal Integrity**: PASS  
  Scope aligned to this repo baseline and Windows/MSYS2 maintained toolchain.

No constitutional violations require complexity exemptions.

## Project Structure

### Documentation (this feature)

```text
specs/001-baseline-replay-verification/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── replay-verification-contract.md
└── tasks.md
```

### Source Code (repository root)

```text
sopwith3/
├── src/
│   ├── sopwith.cpp
│   ├── object.cpp
│   ├── plane.cpp
│   ├── bomb.cpp
│   ├── target.cpp
│   ├── soundsys.cpp
│   └── ...
├── docs/
│   ├── phase2-replay-model-decision.md
│   └── replay-usage.md (to be added/updated by implementation)
└── sopwith3.exe

specs/001-baseline-replay-verification/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
└── contracts/replay-verification-contract.md
```

**Structure Decision**: Use existing single-project native code layout under `sopwith3/src` and keep
feature design artifacts isolated in `specs/001-baseline-replay-verification`.

## Phase 0 - Research Outcome

See `specs/001-baseline-replay-verification/research.md`.

All prior ambiguities are resolved with these decisions:
- Schema mismatch is a hard comparator failure.
- Truncated artifact tails compare only through last complete frame, with truncation warning.
- Missing required row kind is a hard comparator failure.
- Visual playback validation is included in this feature acceptance.

## Phase 1 - Design Outcome

- Data model defined in `specs/001-baseline-replay-verification/data-model.md`
- Interface/contract document defined in `specs/001-baseline-replay-verification/contracts/replay-verification-contract.md`
- Validation execution guide defined in `specs/001-baseline-replay-verification/quickstart.md`

## Post-Design Constitution Re-Check

- Principle I: PASS
- Principle II: PASS
- Principle III: PASS
- Principle IV: PASS
- Principle V: PASS

Re-check result: PASS. No additional constitutional risk introduced by Phase 1 artifacts.

## Complexity Tracking

No constitutional violations or complexity exceptions identified.
