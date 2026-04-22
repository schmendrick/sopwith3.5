# Contract: Phase 2B Determinism Prerequisites

## Purpose

Define the required validation and evidence contract for RNG and fixed-step determinism hardening in
single-player baseline scope.

## Fixture Set Contract

The following fixtures are mandatory for this feature:

- `full.tape`
- `bomb.tape`
- `bird.tape`
- `computer.tape`

No fixture may be omitted from completion evidence.

## Repeatability Execution Contract

For each fixture and each phase (`pre_change`, `post_change`):

1. Execute at least two equivalent runs.
2. Produce two sidecar artifacts from those runs.
3. Compare artifacts under the existing replay comparison contract.
4. Record pass/fail outcome and divergence summary.

## Determinism Evidence Contract

Each fixture must produce:

- pre-change status (`pass` or `fail`)
- post-change status (`pass` or `fail`)
- delta class:
  - `pass_to_pass`
  - `fail_to_pass`
  - `pass_to_fail`
  - `fail_to_fail`

`pass_to_fail` is a blocking regression outcome for this phase.

## Scope Contract

- Applies only to single-player baseline determinism hardening.
- Does not expand feature scope to multiplayer parity or cross-platform parity.
- Fixed-step validation is for deterministic verification, not gameplay redesign.

## Reporting Contract

Phase completion report must include:

1. Fixture-by-fixture before/after table or equivalent structured summary.
2. Explicit list of regressions and improvements.
3. Notes for any unresolved determinism edge case and planned follow-up.
