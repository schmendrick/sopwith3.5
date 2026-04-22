# Implementation Plan: Phase 2B Determinism Prerequisites

**Branch**: `003-phase2b-determinism-prereqs` | **Date**: 2026-04-22 | **Spec**: `specs/003-phase2b-determinism-prereqs/spec.md`  
**Input**: Feature specification from `specs/003-phase2b-determinism-prereqs/spec.md`

## Summary

Establish and execute a deterministic replay hardening plan for the single-player baseline using the
fixture matrix (`full.tape`, `bomb.tape`, `bird.tape`, `computer.tape`). Work proceeds in sequence:
capture pre-change repeatability evidence, harden RNG behavior, validate timer-independent fixed-step
replay behavior, and compare post-change fixture outcomes against baseline with explicit per-fixture
pass/fail reporting.

## Technical Context

**Language/Version**: C++ (legacy Sopwith 3 codebase), PowerShell scripting for validation workflow  
**Primary Dependencies**: SDL 1.2 runtime path, existing replay/history path in `sopwith.cpp`, `replay-compare` tool, replay verification scripts  
**Storage**: Filesystem artifacts (`*.tape`, `*.sidecar`) and checked-in evidence docs under `specs/003-phase2b-determinism-prereqs/`  
**Testing**: Existing replay tests and scripted sidecar comparisons; deterministic repeat-run checks across fixture matrix  
**Target Platform**: Windows 11 + MSYS2 MinGW64 maintained path  
**Project Type**: Native desktop game with CLI replay tooling and spec-driven validation workflow  
**Performance Goals**: Preserve practical replay validation speed for four-fixture matrix in local maintainer workflow  
**Constraints**: Preserve gameplay baseline while changing determinism touchpoints; keep single-player scope; maintain canonical replay artifact diagnostics  
**Scale/Scope**: Four fixture tapes, single-player baseline only, no multiplayer expansion in this phase

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- **Principle I — Preserve gameplay baseline while hardening determinism**: PASS — scope targets deterministic behavior controls without gameplay redesign.
- **Principle II — Replay artifacts + first divergence are canonical**: PASS — fixture-based sidecar comparison remains the central diagnostic mechanism.
- **Principle III — Verification gates and reproducible evidence**: PASS — pre/post repeatability evidence is mandatory for every fixture.
- **Principle IV — Stable identity/ordering/logical-frame contracts**: PASS — fixed-step work explicitly reinforces logical-frame determinism assumptions.
- **Principle V — Scope discipline/toolchain/license integrity**: PASS — work remains on Windows/MSYS2 single-player baseline and existing repository boundaries.

No justification entries required in Complexity Tracking.

## Project Structure

### Documentation (this feature)

```text
specs/003-phase2b-determinism-prereqs/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── determinism-prerequisites-contract.md
└── tasks.md             # created by /speckit.tasks
```

### Source Code (areas touched)

```text
sopwith3/src/
├── sopwith.cpp          # RNG seeding/timer loop/fixed-step touchpoints
├── object.cpp           # canonical randv consumption path
├── plane.cpp            # RNG consumer behavior
├── bomb.cpp             # RNG consumer behavior
├── target.cpp           # RNG consumer behavior
└── soundsys.cpp         # direct libc rand() isolation/removal path

sopwith3/scripts/replay/
├── verify-baseline.ps1  # artifact comparison workflow
└── run-tests.ps1        # scripted replay validation entrypoints

sopwith3/tests/replay/
└── *.cpp                # repeatability and determinism regression checks
```

**Structure Decision**: Keep implementation in the existing native `sopwith3/` project; keep planning, research, and contracts isolated in `specs/003-phase2b-determinism-prereqs/`.

## Phase 0 — Research Outcome

Research resolves the determinism policy decisions needed for implementation ordering:
baseline capture protocol, RNG source unification rules, fixed-step validation boundaries, and evidence
reporting format for fixture-level before/after outcomes.

## Phase 1 — Design Outcome

| Artifact | Purpose |
|---------|---------|
| `data-model.md` | Defines fixtures, runs, comparison outcomes, and evidence-set relationships |
| `contracts/determinism-prerequisites-contract.md` | Defines the required validation protocol and pass/fail reporting contract |
| `quickstart.md` | Defines the operator sequence to capture baseline and post-change results |

Agent context marker in `.cursor/rules/specify-rules.mdc` is updated to this plan.

## Phase 2 — Implementation Planning *(stop here for /speckit.plan)*

Workstreams for `/speckit.tasks` and implementation:

1. Capture pre-change repeatability results for each fixture (`full`, `bomb`, `bird`, `computer`).
2. Harden RNG determinism in primary seeding and consumption touchpoints (`sopwith.cpp`, `object.cpp`, `plane.cpp`, `bomb.cpp`, `target.cpp`).
3. Remove or isolate direct libc `rand()` use from simulation-critical behavior (`soundsys.cpp` classification path).
4. Add timer-independent fixed-step replay validation mode and integrate with fixture matrix checks.
5. Record post-change fixture outcomes and document regressions/improvements relative to baseline.

Current execution note: step 1 is already confirmed complete by maintainer evidence; start implementation at
steps 2-4, then finish with step 5.

## Post-design Constitution Re-check

| Principle | Result |
|-----------|--------|
| I | PASS |
| II | PASS |
| III | PASS |
| IV | PASS |
| V | PASS |

## Complexity Tracking

None.
