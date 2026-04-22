# Implementation Plan: RNG Branch-Decision Parity Harness

**Branch**: `004-rng-branch-parity` | **Date**: 2026-04-22 | **Spec**: `specs/004-rng-branch-parity/spec.md`  
**Input**: Feature specification from `specs/004-rng-branch-parity/spec.md`

## Summary

Extend the existing C++/C# RNG-core parity harness to validate deterministic RNG-driven branch decisions
(troubled sound bit and explosion-type derivations), maintain byte-identical cross-language output,
and update replay parity evidence with branch-level pass/fail coverage.

## Technical Context

**Language/Version**: C++ (native harness), C# (`net10.0`) parity harness, PowerShell orchestration  
**Primary Dependencies**: existing replay token normalization logic, frozen RNG core contract, parity runner script, .NET 10 SDK, MSYS2 MinGW64 toolchain  
**Storage**: file-based harness outputs and evidence docs under feature specs; no new persistent service storage  
**Testing**: byte-comparison parity run via single script with token/step matrices; mismatch location visibility is mandatory  
**Target Platform**: Windows 11 maintainer workflow  
**Project Type**: native + managed CLI parity harness tooling in a single repository  
**Performance Goals**: branch-parity suite completes quickly for routine local verification (same order as existing core parity checks)  
**Constraints**: preserve existing RNG-core parity behavior; do not expand into full gameplay parity; keep output machine-diffable  
**Scale/Scope**: branch-level deterministic decisions only (troubled sound bit + explosion type), no multiplayer parity expansion

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- **Principle I — Preserve gameplay baseline while hardening determinism**: PASS — harness-only parity extension, no gameplay redesign.
- **Principle II — Replay artifacts and diagnostics are canonical**: PASS — branch-decision outputs are deterministic diagnostic artifacts.
- **Principle III — Verification gates and reproducible evidence**: PASS — explicit cross-language parity gate with reproducible command.
- **Principle IV — Stable identity/ordering/logical-frame contracts**: PASS — extension remains in deterministic contract space and does not alter frame contract semantics.
- **Principle V — Scope discipline/toolchain/license integrity**: PASS — keeps Windows/MSYS2/SDL and .NET toolchain reality explicit; scope remains branch-level parity.

No complexity exceptions required.

## Project Structure

### Documentation (this feature)

```text
specs/004-rng-branch-parity/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── branch-parity-contract.md
└── tasks.md
```

### Source Code (repository root)

```text
sopwith3/src/
├── rng_parity_tool.cpp
└── Makefile.msys2

parity-harness/
├── run-rng-parity.ps1
└── csharp/
   ├── RngParityHarness.csproj
   └── Program.cs

specs/003-phase2b-determinism-prereqs/
└── evidence.md (extended with branch-parity evidence)
```

**Structure Decision**: Reuse existing RNG-core harness structure and extend outputs/contracts incrementally rather than introducing new tooling surfaces.

## Phase 0 — Research Outcome

Research resolves deterministic branch-decision contract details:

1. exact troubled-sound-bit derivation mapping
2. exact explosion-type branch derivation inputs and arithmetic boundaries
3. output schema extension strategy that preserves byte-identity comparison friendliness

## Phase 1 — Design Outcome

| Artifact | Purpose |
|---------|---------|
| `data-model.md` | Defines branch parity case/result entities and mismatch reporting shape |
| `contracts/branch-parity-contract.md` | Defines required branch outputs, pass/fail semantics, and output schema |
| `quickstart.md` | Defines reproducible branch-parity execution and evidence update flow |

Agent context pointer in `.cursor/rules/specify-rules.mdc` is updated to this feature plan.

## Phase 2 — Implementation Planning *(stop here for /speckit.plan)*

Implementation workstreams for `/speckit.tasks`:

1. Extend C++ parity tool output to include branch-decision fields.
2. Extend C# parity tool output with identical semantics and formatting.
3. Update parity runner to validate new branch fields and preserve clear mismatch reporting.
4. Expand case matrix and evidence updates for branch-level outcomes.
5. Validate documentation/repro remains accurate for current toolchain requirements.

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
