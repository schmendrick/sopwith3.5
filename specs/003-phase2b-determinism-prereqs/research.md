# Research: Phase 2B Determinism Prerequisites

## Baseline Status

- **Maintainer confirmation**: Pre-change repeatability runs were already executed for `full.tape`,
  `bomb.tape`, `bird.tape`, and `computer.tape`, and baseline outcomes are confirmed.
- **Planning impact**: Phase 2B implementation starts at RNG hardening and fixed-step validation;
  baseline capture is treated as completed evidence.

## Decision 1: Fixture Matrix Is Normative

- **Decision**: Use `full.tape`, `bomb.tape`, `bird.tape`, and `computer.tape` as required fixtures for all determinism checks in this phase.
- **Rationale**: The set covers broad gameplay, explosion behavior, flock/order effects, and vs-computer behavior with practical run volume.
- **Alternatives considered**:
  - Single long fixture only (rejected: misses path-specific nondeterminism)
  - Ad hoc fixture choice per run (rejected: weak comparability across changes)

## Decision 2: Before/After Evidence Is Mandatory

- **Decision**: Every fixture must have pre-change and post-change repeatability outcomes recorded.
- **Rationale**: Determinism hardening requires objective deltas, not anecdotal observations.
- **Alternatives considered**:
  - Post-change-only reporting (rejected: no regression/improvement baseline)
  - Spot-check only one fixture after changes (rejected: insufficient coverage)

## Decision 3: RNG Hardening Order

- **Decision**: Address deterministic seed policy and RNG source consistency before fixed-step validation changes.
- **Rationale**: RNG divergence can invalidate fixed-step conclusions if left unresolved.
- **Alternatives considered**:
  - Fixed-step first, RNG later (rejected: attribution of failures becomes ambiguous)
  - Simultaneous broad refactor (rejected: high diagnostic complexity)

## Decision 4: Direct `rand()` Handling Policy

- **Decision**: Direct libc `rand()` in runtime paths must be removed from simulation-critical behavior or explicitly isolated to non-simulation effects.
- **Rationale**: Mixed RNG sources are a known determinism risk.
- **Alternatives considered**:
  - Keep mixed RNG and document caveat (rejected: violates deterministic intent)
  - Replace all randomness globally in one step (rejected: unnecessary blast radius for this phase)

## Decision 5: Fixed-Step Validation Scope

- **Decision**: Fixed-step mode is a test/validation path for deterministic replay verification, not a gameplay mode change.
- **Rationale**: Maintains gameplay baseline while enabling timer-independent deterministic checks.
- **Alternatives considered**:
  - Replace standard loop behavior globally (rejected: gameplay-risky for this phase)
  - Skip fixed-step and rely on wall-clock path only (rejected: unresolved timer nondeterminism)

## Decision 6: Regression Classification

- **Decision**: Outcomes are classified per fixture as `pass->pass`, `fail->pass`, `pass->fail`, or `fail->fail`.
- **Rationale**: This gives maintainers immediate clarity on regressions and improvements.
- **Alternatives considered**:
  - Binary project-level status only (rejected: hides fixture-level behavior)
  - Raw compare output without normalized status labels (rejected: slower triage)
