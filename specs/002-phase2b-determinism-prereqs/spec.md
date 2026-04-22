# Feature Specification: Phase 2B Determinism Prerequisites

**Feature Branch**: `002-phase2b-determinism-prereqs`  
**Created**: 2026-04-22  
**Status**: Draft  
**Input**: Roadmap phase "Phase 2B determinism prerequisites (RNG/timer hardening + one-step simulation API)".

## Objective

Harden deterministic behavior for single-player replay baseline before further replay/parity expansion.

## Determinism Fixture Set (Normative)

The following replay tapes are the required fixture matrix for Phase 2B validation:

- `full.tape`: broad single-player coverage and general progression.
- `bomb.tape`: explosion and blast behavior paths.
- `bird.tape`: flock and object-order-sensitive behavior paths.
- `computer.tape`: vs-computer AI behavior paths.

## Validation Protocol (Normative)

For each fixture tape:

1. Execute at least two runs with identical CLI flags and seed behavior assumptions.
2. Compare generated sidecars for repeat-run equality (`basename.1.sidecar` vs `basename.2.sidecar`).
3. Record result as pass/fail in both:
   - pre-change baseline (before RNG/timer modifications)
   - post-change verification (after each determinism hardening change-set)

## Functional Requirements

- **FR-001**: System MUST define and use a portable deterministic RNG specification for the baseline simulation path.
- **FR-002**: System MUST eliminate or explicitly isolate direct libc RNG (`rand`) from simulation-critical behavior.
- **FR-003**: System MUST provide a test-only one-step simulation API independent of wall-clock catch-up logic.
- **FR-004**: System MUST preserve baseline gameplay behavior while improving replay determinism.
- **FR-005**: System MUST pass repeat-run equality across all four fixture tapes in post-change verification.

## Success Criteria

- **SC-001**: All four fixtures (`full`, `bomb`, `bird`, `computer`) pass repeat-run sidecar equality post-change.
- **SC-002**: Any fixture that failed pre-change and passes post-change is explicitly documented as a determinism improvement.
- **SC-003**: No new deterministic regressions are introduced in previously passing fixtures.
