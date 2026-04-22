# Tasks: Phase 2B Determinism Prerequisites

**Input**: Artifacts from `specs/003-phase2b-determinism-prereqs/`  
**Prerequisites**: `spec.md`, `plan.md`, `research.md`, `data-model.md`, `contracts/`  
**Tests**: Required (fixture repeatability comparisons pre/post change)

## Status Note

Maintainer confirmed pre-change baseline capture for all four fixtures; baseline tasks are marked complete.

## Format: `[ID] [P?] Description`

- **[P]**: Parallelizable (different files/no direct dependency)

## Phase 1: Baseline Capture (Completed)

- [X] T001 Capture pre-change repeatability outcome for `full.tape`.
- [X] T002 Capture pre-change repeatability outcome for `bomb.tape`.
- [X] T003 Capture pre-change repeatability outcome for `bird.tape`.
- [X] T004 Capture pre-change repeatability outcome for `computer.tape`.

## Phase 2: RNG Hardening (Active)

- [X] T005 Implement deterministic seed policy update in `sopwith3/src/sopwith.cpp` for replay validation paths.
- [X] T006 [P] Audit and normalize canonical `randv` consumption expectations in `sopwith3/src/object.cpp`, `sopwith3/src/plane.cpp`, `sopwith3/src/bomb.cpp`, and `sopwith3/src/target.cpp`.
- [X] T007 Remove or isolate direct libc `rand()` from simulation-critical behavior in `sopwith3/src/soundsys.cpp`.

## Phase 3: Fixed-Step Validation Path

- [X] T008 Add timer-independent fixed-step replay validation path in `sopwith3/src/sopwith.cpp`.
- [X] T009 [P] Extend replay validation test/script workflow for fixed-step runs in `sopwith3/scripts/replay/`.

## Phase 4: Post-change Verification and Evidence

- [X] T010 Re-run fixture repeatability comparisons for `full.tape` and record delta.
- [X] T011 Re-run fixture repeatability comparisons for `bomb.tape` and record delta.
- [X] T012 Re-run fixture repeatability comparisons for `bird.tape` and record delta.
- [X] T013 Re-run fixture repeatability comparisons for `computer.tape` and record delta.
- [X] T014 Publish fixture-level evidence summary (`pass_to_pass`, `fail_to_pass`, `pass_to_fail`, `fail_to_fail`) in this feature directory.

## Execution Order

1. Complete RNG hardening (T005-T007).
2. Complete fixed-step path (T008-T009).
3. Run post-change verification and publish evidence (T010-T014).
