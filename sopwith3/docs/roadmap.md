# Sopwith 3.5 Roadmap

## Purpose

This repository is the **Sopwith 3.5 C++ foundation** project.  
Its job is to make Sopwith 3 build reliably, become deterministic/testable, and produce AI-verifiable outputs for future parity checks.

The **C# port is explicitly out of scope for this repository** and will happen later in a separate repo.

## Scope

### In Scope (this repo)

- Build and run Sopwith 3 on modern Windows (MSYS2 + SDL1.2 path).
- Deterministic replay/testing improvements on top of existing `-h` (record) and `-v` (playback).
- Non-visual ("headless") stepping and structured golden-state outputs for automated comparison.
- Monitoring/logging to diagnose behavior changes.
- Phase 0 feasibility memo:  
  _"Can Sopwith 3 be made deterministically replayable with less than 40 hours of C++ refactoring effort?"_

### Out of Scope (this repo)

- Production C# gameplay implementation.
- Large gameplay feature additions before deterministic baseline is stable.
- Network code remains preserved as an optional objective for the later C# port, but is not part of this repo's tested baseline.

## Baseline Gameplay to Preserve

- Single (expert), novice, and vs-computer behavior.
- Existing game feel, controls, sprites, and current map.
- Initial parity scenario for tests: **single-player only**.

## Key Terms (used below)

- **Replay tape:** A saved recording of player inputs plus seed/metadata used to reproduce a run.
- **Headless mode:** Run simulation without rendering to a visible game window (for automated tests).
- **Golden-state output:** Structured per-frame state dump used as the reference when comparing runs (for example, C++ baseline vs later C#).
- **Logical frame:** One simulation step (the unit counted by `framecounter`), independent of wall-clock time.

## Current Status

- **Phase 0:** Completed with **CONDITIONAL GO** (see `docs/phase0-deterministic-replay-memo.md`).
- **Phase 1:** Manual smoke baseline verified (build, launch, controls, clean quit path).
- **Next active work:** Phase 2A replay-model decision doc at `docs/phase2-replay-model-decision.md`.

## Phases

## Phase 0 — Feasibility Memo

- Audit current replay, RNG, timing, and order-dependent behavior.
- Estimate effort for deterministic replay hooks.
- Deliver clear verdict: **go / conditional-go / no-go** for `< 40h`.

**Exit criteria**
- Memo checked in at `docs/phase0-deterministic-replay-memo.md`.
- Contains evidence, assumptions, estimate range, and recommendation.

## Phase 1 — Build & Run Reliability

1. [x] Reproducible Windows build instructions for SDL                
2. [x] Confirm binary startup and asset-path expectations for SDL
3. [x] Reproducible Windows build instructions for Allegro                
4. [f] Confirm binary startup and asset-path expectations for Allegro
5. [ ] Remove Allegro and DJGPP code
6. [ ] Reproducible Windows build instructions for building with SDL + Network code (build but untested)                

**Exit criteria**
- Fresh machine can build and run from documented steps for BOTH SDL and Allegro and network
- Manual smoke test includes launch, basic input sanity, and a verified way to quit without force-killing the process.


## Phase 2 — Replay Design + Determinism Prerequisites

- **Phase 2A (analysis/brainstorming):** evaluate replay model options and select one with explicit tradeoffs.
  - Compare formats: compact text (TSV/columnar) vs binary(+decoder) vs hybrid.
  - Decide capture granularity: every frame vs sampled frames; full state vs targeted entity fields.
  - Define stable identity/order for entities in dumps (so diffs are meaningful).
  - Define replay consumption modes: **headless verification replay** (for AI/tests) and **visual playback replay** (for humans to inspect a saved run).
  - Estimate file size/runtime overhead and choose default test profile.
- **Phase 2B (determinism prerequisites):** harden determinism enough for trustworthy replay baselines.
  - Audit RNG and wall-clock dependencies, then **refactor the affected code touchpoints** (e.g. seeding/loop/timer/RNG call-sites) so simulation is deterministic for test replay.
  - Define, document, and implement a **portable deterministic RNG specification in C++** (algorithm + seed/overflow/call-order rules) so it can be mirrored later in the separate C# port.
  - Provide a test-only simulation step API that advances the game by exactly one logic frame per call (`framecounter += 1`), independent of wall-clock time (no SDL timer catch-up/backlog loops).
  - Confirm repeatability: same input+seed gives same state output across repeated runs.
  - Track and execute deterministic refactors in these primary touchpoints:
    - `src/sopwith.cpp` (seeding, main loop pacing, history/replay wiring)
    - `src/object.cpp` (canonical `randv` update/consumption path)
    - `src/soundsys.cpp` (direct `rand()` usage and classification/refactor)
    - RNG consumer call-sites in `src/plane.cpp`, `src/bomb.cpp`, `src/target.cpp`

**Exit criteria**
- `docs/phase2-replay-model-decision.md` checked in with options considered, chosen model, and rationale.
- Determinism checklist completed for the chosen single-player baseline.
- Headless test mode can advance an explicitly chosen frame count (`N`) and produces identical state output across at least two repeated runs with the same input+seed.

## Phase 3 — Replay Implementation + Golden Outputs

- Implement replay pipeline on top of the hardened deterministic path.
- Keep replay-tape + seed replay model.
- Add headless one-step simulation path.
- Ensure a saved replay can be played back visually in-game (existing `-v` flow or improved equivalent).
- Export structured per-frame state (chosen schema from Phase 2A).
- Document replay usage (CLI params, file formats, and copy-paste examples).

**Exit criteria**
- One scripted single-player replay produces stable, diff-friendly output across repeated runs.
- At least one saved replay can be opened and watched end-to-end for manual inspection.
- Replay documentation exists with practical command lines for record/playback/headless verification.

## Phase 4 — Monitoring & Regression Workflow

- Add clear event/state diagnostics for investigation.
- Make automated replay checks easy to run in CI/local scripts.

**Exit criteria**
- A single command can run replay verification and report first divergence.

## Working Principles

- Prefer transparent, diffable test artifacts over opaque pass/fail signals.
- Use deterministic logical-frame stepping for parity checks.
- Keep changes incremental and measurable.
- Preserve GPL obligations and attribution.

## Planned Companion Documents

- `docs/phase0-deterministic-replay-memo.md` (first deliverable)
- `docs/phase1-build-run-smoke.md` (build/run/manual-smoke evidence)
- `docs/phase2-replay-model-decision.md` (Phase 2A replay model decision record)
- `docs/replay-usage.md` (CLI flags, example commands, expected outputs, troubleshooting)
- `docs/adr/0001-deterministic-replay-approach.md` (optional, if ADR style is used)
