# Tasks: Phase 2B Determinism Prerequisites

**Input**: Design artifacts in `specs/002-phase2b-determinism-prereqs/`  
**Tests**: Required; determinism work must prove repeatability across the fixture matrix.

## Determinism Fixture Matrix

- `full.tape`
- `bomb.tape`
- `bird.tape`
- `computer.tape`

## Format: `[ID] [P?] Description`

- **[P]**: Can run in parallel (different files, no direct dependency)

## Phase 1: Baseline Capture

- [ ] T001 Record pre-change repeatability results for `full.tape` (2 identical runs + sidecar compare).
- [ ] T002 Record pre-change repeatability results for `bomb.tape` (2 identical runs + sidecar compare).
- [ ] T003 Record pre-change repeatability results for `bird.tape` (2 identical runs + sidecar compare).
- [ ] T004 Record pre-change repeatability results for `computer.tape` (2 identical runs + sidecar compare).

## Phase 2: RNG Hardening

- [ ] T005 Replace wall-clock seed initialization in `sopwith3/src/sopwith.cpp` with deterministic seed policy for test replay mode.
- [ ] T006 Remove or isolate direct libc `rand()` use in `sopwith3/src/soundsys.cpp` from simulation-critical determinism path.
- [ ] T007 Confirm and document canonical RNG call-order expectations for simulation touchpoints (`sopwith3/src/object.cpp`, `sopwith3/src/plane.cpp`, `sopwith3/src/bomb.cpp`, `sopwith3/src/target.cpp`).

## Phase 3: Timer Independence

- [ ] T008 Add test-only one-step simulation entrypoint independent of realtime timer accumulation in `sopwith3/src/sopwith.cpp`.
- [ ] T009 Add/extend repeatability test harness to run all four fixtures under fixed-step conditions.

## Phase 4: Verification and Closeout

- [ ] T010 Re-run repeatability checks for `full.tape` and compare against baseline.
- [ ] T011 Re-run repeatability checks for `bomb.tape` and compare against baseline.
- [ ] T012 Re-run repeatability checks for `bird.tape` and compare against baseline.
- [ ] T013 Re-run repeatability checks for `computer.tape` and compare against baseline.
- [ ] T014 Update determinism documentation with fixture results and any remaining known nondeterministic edges.
