# Implementation Plan: Phase 2C Gameplay Parity Surface

**Branch**: `006-gameplay-parity-surface` | **Date**: 2026-04-22 | **Spec**: `specs/006-gameplay-parity-surface/spec.md`  
**Input**: Feature specification from `specs/006-gameplay-parity-surface/spec.md`

## Summary

Add a **new** cross-language parity harness surface (separate from `rng-parity-cpp`) that emits deterministic **gameplay decision records** and **replay writer entity ordering** diagnostics for a **versioned** token/step case matrix, with **byte-identical** C++/C# stdout and **first mismatch** reporting.

Preserve existing Phase 2B/004 RNG parity contracts and tooling outputs unchanged.

## Technical Context

**Language/Version**: C++ (native harness executable), C# (`net10.0`) parity harness, PowerShell orchestration  
**Primary Dependencies**: existing replay path normalization (`replay_paths.*`), replay writer entity ordering helpers (`replay_writer_entities.*`), MSYS2 MinGW64 toolchain, .NET 10 SDK  
**Storage**: file-based harness stdout artifacts + spec evidence markdown under `specs/006-gameplay-parity-surface/`  
**Testing**: byte-comparison via PowerShell runner (mirrors `tools/run-rng-parity.ps1` behavior)  
**Target Platform**: Windows 11 maintainer workflow (byte parity intentionally scoped here per spec clarifications)  
**Project Type**: native + managed CLI parity harness tooling in a single repository  
**Performance Goals**: gameplay parity matrix completes in “routine local verification” time (same order of magnitude as RNG parity runs)  
**Constraints**: no `sopwith3.exe` gameplay behavior changes as part of this feature; RNG parity outputs/contracts must remain stable; outputs must remain machine-diffable  
**Scale/Scope**: selected gameplay decision categories only (spawn/lifecycle, collision outcome v1 transitions, score/lives deltas, replay entity ordering)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- **Principle I — Preserve gameplay baseline while hardening determinism**: PASS — harness-only extension; avoids gameplay redesign and avoids changing `sopwith3.exe` behavior per spec.
- **Principle II — Replay artifacts and diagnostics are canonical**: PASS — emits deterministic, line-oriented parity artifacts with explicit first mismatch context.
- **Principle III — Verification gates and reproducible evidence**: PASS — defines reproducible runner + evidence update targets; matrix/tape identity captured in headers.
- **Principle IV — Stable identity, ordering, and logical-frame contracts**: PASS — explicitly targets deterministic ordering keys and stable record ordering rules; must not alter replay frame contract semantics.
- **Principle V — Scope discipline, toolchain reality, and license/attribution integrity**: PASS — stays within Windows/MSYS2/SDL + .NET maintainer reality; bounded scope.

No complexity exceptions required.

## Project Structure

### Documentation (this feature)

```text
specs/006-gameplay-parity-surface/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── gameplay-parity-contract.md
└── tasks.md                 # created by /speckit.tasks (not created here)
```

### Source Code (repository root)

```text
sopwith3/src/
├── Makefile.msys2                 # add gameplay parity tool target (modeled after rng-parity-cpp)
├── replay_paths.cpp             # reuse token/tape normalization
├── replay_writer_entities.cpp # ordering + payload sources of truth for replay parity surface

sopwith3/tools/
├── rng_parity_tool.cpp          # unchanged contract surface (Phase 2B/004)
└── (new) gameplay_parity_tool.cpp

tools/
├── run-rng-parity.ps1           # existing RNG runner (must keep working)
├── (new) run-gameplay-parity.ps1
└── csharp/
    ├── RngParityHarness.csproj  # existing
    ├── Program.cs               # existing RNG harness
    └── (new) GameplayParityHarness/ (or sibling project) for parity port
```

**Structure Decision**: Add sibling harnesses and a sibling runner rather than extending RNG stdout, preserving Phase 2B/004 byte contracts while enabling additive Phase 2C outputs.

## Phase 0 — Research Outcome

Captured in `specs/006-gameplay-parity-surface/research.md`.

Key outcomes:

- Dedicated gameplay parity harness (do not overload `rng-parity-cpp`)
- Runner mirrors `run-rng-parity.ps1` ergonomics (`first_mismatch ...`)
- Matrix/tape evolution handled via `matrix_version` + explicit `tape_ref` header lines

## Phase 1 — Design Outcome

| Artifact | Purpose |
|---------|---------|
| `data-model.md` | Entities: versioned matrix, cases, tape refs, decision records, mismatch tuple |
| `contracts/gameplay-parity-contract.md` | Output schema, ordering rules, pass/fail + mismatch reporting |
| `quickstart.md` | Repro commands + evidence update expectations |

Agent context pointer in `.cursor/rules/specify-rules.mdc` is updated to this feature plan.

## Phase 2 — Implementation Planning *(stop here for /speckit.plan)*

Implementation workstreams for `/speckit.tasks`:

1. Add `gameplay-parity-cpp` build target + minimal `main` that runs a deterministic scenario for N logical steps and prints contract-compliant stdout.
2. Add C# harness project mirroring the C++ stdout formatting and decision extraction.
3. Add `run-gameplay-parity.ps1` modeled on `run-rng-parity.ps1` (matrix, builds, byte compare, first mismatch).
4. Add versioned matrix file + initial cases mapped to existing tokens/tapes (starting from current RNG defaults where appropriate).
5. Add `evidence.md` under this feature directory + link from Phase 2B evidence rollup if desired.
6. Regression gate: `run-rng-parity.ps1` remains green and RNG outputs unchanged.

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
