# Phase 0 Technical Memo: Deterministic Replay Feasibility

## Decision Question

Can Sopwith 3 be made deterministically replayable with **less than 40 hours** of C++ refactoring effort?

## Context

- Repository scope: Sopwith 3.5 C++ foundation work in `sopwith3/` (build reliability, determinism, replay, observability).
- Goal of this memo: provide a practical go/no-go recommendation for the determinism+replay baseline needed before the later C# port.
- Baseline parity scenario: single-player.

## Current Evidence (from existing code)

### Existing replay capabilities

- Current flags: `-h` (record) and `-v` (playback).
- What is currently recorded (known):
  - CLI help exposes `-h*` and `-v*` in `src/sopwith.cpp`.
  - `inithistory()` reads/writes initial `randv` and history metadata.
  - `history()` records `(framecounter, keys)` changes and applies playback by frame number.
- What is currently missing for parity-grade verification:
  - [ ] test-only one-frame stepping API
  - [ ] structured golden-state output
  - [ ] replay usage docs for repeatable CLI-driven runs

### Determinism risk areas

- RNG sources (custom `randv` path and any stray `rand()` usage):
  - Findings:
    - `run()` seeds with `randv=time(0); srand(randv)` in `src/sopwith.cpp`.
    - `randv` is also consumed/advanced in game logic (`src/object.cpp`).
    - At least one direct libc `rand()` call exists in `src/soundsys.cpp`.
  - Impact:
    - Mixed RNG sources increase risk of nondeterministic behavior across runs/platforms.
    - Direct `rand()` usage should be isolated or replaced for strict replay parity.
- Wall-clock dependence in simulation path:
  - Findings:
    - Main loop depends on timer accumulation (`processtimerticks`, `speedtick`, `timer()` in `src/sopwith.cpp`).
    - Update pacing currently depends on realtime catch-up behavior.
  - Impact:
    - Same input tape can be sensitive to timing/pacing differences unless test mode uses fixed logical stepping.
- Order-dependent update/collision behavior:
  - Findings:
    - Object updates iterate `objectlist` in insertion/list order (`updateobjects`).
    - Collision pass sorts a copy using a stable sort (`checkcollisions`).
  - Impact:
    - Mostly deterministic if insertion and sorting rules remain stable, but this must be verified with repeatability tests.

## Candidate Approaches

## RNG Touchpoint Checklist (for implementation tracking)

**Portability requirement (explicit):**

- The RNG refactor in this repository must define a **portable deterministic RNG specification** (algorithm, seed rules, integer width/overflow behavior, and call-order expectations) so the same behavior can be implemented in the later C# port.

- Core seed/setup
  - [ ] `src/sopwith.cpp` (`run()`): `randv=time(0); srand(randv);` reviewed/refactored for deterministic test mode.
- Core custom RNG path
  - [ ] `src/object.cpp`: `randv` update/consumption logic validated and documented as canonical RNG sequence.
- Direct libc RNG calls
  - [ ] `src/soundsys.cpp`: direct `rand()` usage reviewed (isolate, replace, or explicitly classify as non-simulation-only randomness).
- Gameplay consumers depending on `randv`
  - [ ] `src/plane.cpp` (explosion/behavior branches using `randv`)
  - [ ] `src/bomb.cpp` (explosion behavior using `randv`)
  - [ ] `src/target.cpp` (explosion/behavior branches using `randv`)
- Optional/network-related RNG paths (deferred from tested baseline but should be noted)
  - [ ] `src/hawknl/network.h` (`random_shuffle` usage)
  - [ ] `src/libnet/network.h` (`random_shuffle` usage)

## Option A: Minimal deterministic baseline (recommended default candidate)

- Scope:
  - deterministic stepping path for tests (one logical frame per call)
  - controlled RNG behavior
  - structured per-frame state output for selected entities/fields
- Pros:
  - likely within time budget
  - directly supports AI/human diff diagnostics
- Cons:
  - not a full binary dump of every object field

## Option B: Full per-frame full-object snapshot

- Scope:
  - serialize complete state for all objects every frame
- Pros:
  - maximal forensic detail
- Cons:
  - larger implementation and file-size/runtime overhead
  - higher risk of exceeding 40h

## Estimated Effort

### Work breakdown estimate (hours)

- Determinism audit + touchpoint refactor: 8-12
- Test-only frame-step API: 4-6
- Replay/golden-state schema + export: 8-12
- Manual validation + repeatability checks: 4-6
- Documentation (`docs/replay-usage.md` + memo updates): 2-3
- Buffer/contingency: 3-5

**Total estimate:** `29-44` hours (most likely band: `32-38`)

## Recommendation

- Recommended option: `Option A`
- Rationale:
  - Fits the project objective (diagnosable parity checks) with lower implementation risk than full snapshots.
  - Most likely to stay within the `<40h` target if scope remains focused on single-player baseline and compact structured output.

## Verdict on Decision Question

**Verdict:** `CONDITIONAL GO` for `< 40h`

### If GO or CONDITIONAL GO

- Conditions/assumptions:
  - Scope stays on single-player replay baseline first (no multiplayer/network parity in this phase).
  - Structured output targets key gameplay entities/fields (planes/bombs/bullets/score/counters), not full-object binary snapshots of every field.
- First implementation milestone:
  - Add deterministic test-step path and prove two identical replay runs with same input+seed produce identical structured output.

### If NO-GO

- Primary blockers:
  - `__`
  - `__`
- Alternative scope proposal:
  - `__`

## Evidence Log (fill during investigation)

- Build/run observations:
  - Date:
  - Commands:
  - Notes: Pending manual run/quit-path verification.
- Determinism replay trial #1:
  - Seed/input:
  - Output comparison result:
  - Notes: Pending implementation of test-step path + structured output.
- Determinism replay trial #2:
  - Seed/input:
  - Output comparison result:
  - Notes: Pending implementation of test-step path + structured output.

## Sign-off

- Author: AI-assisted draft (to be finalized by maintainer)
- Date:
- Related docs:
  - `docs/roadmap.md`
  - `docs/phase2-replay-model-decision.md` (to be created in Phase 2A)
  - `docs/replay-usage.md` (to be created in Phase 3)
