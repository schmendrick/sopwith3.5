# Feature Specification: Phase 2B Determinism Prerequisites

**Feature Branch**: `003-phase2b-determinism-prereqs`  
**Created**: 2026-04-22  
**Status**: Draft  
**Input**: User description: "Phase 2B determinism prerequisites: harden RNG determinism and timer-independent fixed-step replay validation for single-player baseline using fixtures full.tape, bomb.tape, bird.tape, computer.tape."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Establish Determinism Baseline (Priority: P1)

As a maintainer, I can run a defined replay fixture set and capture repeatability results before changes, so I can measure determinism regressions and improvements objectively.

**Why this priority**: A trustworthy before/after baseline is required for every later hardening decision.

**Independent Test**: Execute two equivalent runs per fixture and verify whether artifact comparisons are equal; store results per fixture as baseline evidence.

**Acceptance Scenarios**:

1. **Given** the fixture set (`full.tape`, `bomb.tape`, `bird.tape`, `computer.tape`), **When** deterministic baseline capture is executed, **Then** each fixture has a recorded repeatability result.
2. **Given** baseline capture has completed, **When** a maintainer reviews the results, **Then** they can identify which fixtures already pass repeatability and which do not.

---

### User Story 2 - Harden RNG Determinism (Priority: P1)

As a maintainer, I can rely on deterministic random behavior in the baseline replay path, so identical inputs produce stable outputs.

**Why this priority**: Uncontrolled randomness is a primary cause of replay divergence.

**Independent Test**: Re-run the same fixture set after RNG hardening and compare outcomes against baseline capture.

**Acceptance Scenarios**:

1. **Given** two runs with equivalent replay conditions, **When** random behavior is evaluated, **Then** randomness-dependent outcomes are consistent across runs.
2. **Given** fixture results from before and after RNG hardening, **When** outcomes are compared, **Then** determinism gains are clearly attributable and documented.

---

### User Story 3 - Validate Fixed-Step Determinism (Priority: P2)

As a maintainer, I can validate replay repeatability under timer-independent fixed-step progression, so determinism checks are not affected by wall-clock variation.

**Why this priority**: Timer sensitivity can mask or reintroduce nondeterminism even when randomness is controlled.

**Independent Test**: Execute replay validation in fixed-step mode for all fixtures and verify repeat-run artifact equality.

**Acceptance Scenarios**:

1. **Given** fixed-step replay validation is enabled, **When** the same fixture is executed repeatedly, **Then** result artifacts remain consistent run-to-run.
2. **Given** all fixtures complete fixed-step validation, **When** maintainers review outputs, **Then** they can determine readiness for downstream replay/parity phases.

---

### Edge Cases

- What happens when one fixture passes in baseline mode but fails in fixed-step mode?
- How is validation handled when fixture artifacts are incomplete or missing expected outputs?
- What happens when deterministic results differ between two fixtures that exercise similar gameplay behaviors?
- How are repeated runs interpreted when results fluctuate across more than two attempts?

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST define and preserve a normative single-player determinism fixture set containing `full.tape`, `bomb.tape`, `bird.tape`, and `computer.tape`.
- **FR-002**: System MUST capture pre-change repeatability outcomes for each fixture using at least two equivalent runs.
- **FR-003**: System MUST harden random-behavior handling so equivalent replay conditions produce consistent outputs.
- **FR-004**: System MUST validate replay repeatability using timer-independent fixed-step progression for the single-player baseline.
- **FR-005**: System MUST record post-change outcomes for each fixture and compare them against pre-change baseline results.
- **FR-006**: System MUST provide fixture-level pass/fail evidence that allows maintainers to identify determinism regressions and improvements.
- **FR-007**: System MUST keep this phase bounded to single-player baseline validation and defer broader parity scope.

### Key Entities *(include if feature involves data)*

- **Determinism Fixture**: A named replay sample used to evaluate repeatability behavior for the baseline scope.
- **Repeatability Result**: The pass/fail outcome from comparing equivalent replay runs for one fixture.
- **Validation Run**: A single execution of replay verification under baseline or fixed-step conditions.
- **Determinism Evidence Set**: The recorded before/after result collection used for decision-making in subsequent phases.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of fixtures in the defined set have pre-change and post-change repeatability results recorded.
- **SC-002**: Maintainers can determine pass/fail repeatability status for each fixture in one review pass.
- **SC-003**: Post-change validation shows no new repeatability regressions on fixtures that passed in pre-change baseline capture.
- **SC-004**: Any fixture that improves from failing to passing repeatability is explicitly identified and documented.

## Assumptions

- The four listed replay tapes remain available and representative of baseline single-player behavior during this phase.
- Equivalent replay conditions are reproducible enough to support before/after repeatability comparison.
- This phase focuses on deterministic prerequisites and does not expand scope to multiplayer or cross-platform parity.
- Existing replay verification workflows continue to provide the comparison evidence required for fixture outcomes.
