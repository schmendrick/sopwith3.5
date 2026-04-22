# Research: Phase 2C Gameplay Parity Surface

## Decision: Add a dedicated gameplay parity harness (separate from RNG parity)

**Rationale**

- `sopwith3/rng-parity-cpp.exe` is intentionally a tiny RNG-only executable (`sopwith3/tools/rng_parity_tool.cpp`) and is not a viable place to hang gameplay/spawn/collision/score semantics without collapsing the Phase 2B/004 RNG parity boundary.
- Phase 2C needs deterministic *gameplay decision* records and stable ordering semantics; the replay writer already contains explicit deterministic ordering rules for emitted entity rows (see `sopwith3/src/replay_writer_entities.cpp`, especially sorting helpers and `replay_object_less`).
- A dedicated harness keeps Phase 2B/004 contracts stable while allowing Phase 2C outputs to evolve as a new additive contract.

**Alternatives considered**

- **Extend RNG harness outputs in-place**: rejected — mixes concerns and risks accidental changes to RNG parity outputs (violates “preserve Phase 2B/004 contracts” intent).
- **Compare only `sopwith3.exe` runtime logs**: rejected — out of scope per `specs/006-gameplay-parity-surface/spec.md` (`FR-010`); also harder to keep byte-stable across tooling.

## Decision: Mirror the existing parity runner shape (PowerShell matrix + first mismatch)

**Rationale**

- `tools/run-rng-parity.ps1` already implements the maintainer workflow: build both sides, run matrix, compare text output, print `first_mismatch token=... steps=... step=... field=... line=...`.
- Phase 2C should reuse the same operational ergonomics to reduce migration friction.

**Alternatives considered**

- **Rewrite runner in C#**: rejected — unnecessary churn; PowerShell is already the canonical orchestration path in-repo.

## Decision: Record matrix version + referenced tape identities in harness headers

**Rationale**

- The feature spec explicitly requires auditable evolution when tapes change (`FR-011`, clarifications session 2026-04-22).
- Headers should be stable, parseable, and line-oriented to preserve byte comparison and “first mismatch” diagnostics.

**Alternatives considered**

- **Implicit matrix (no version)**: rejected — breaks evidence comparability when tapes evolve.

## Decision: “Collision outcome category” v1 is derived from deterministic replay-visible state transitions

**Rationale**

- There is not yet a single dedicated “collision event row” writer API in `replay_writer.h`; entity payloads already expose replay-visible fields such as plane `collided` and object `life`/`hitcounter` (see `replay_payload_for_object*` in `replay_writer_entities.cpp`).
- A v1 contract can still be objective and migration-useful if categories are defined strictly as transitions on a small set of normalized fields, then expanded later via matrix version bumps.

**Alternatives considered**

- **Full physics collision classifier parity**: deferred — exceeds Phase 2C “surface” intent and couples too tightly to deep gameplay internals.

## Open engineering tasks (for `/speckit.tasks`, not unresolved research)

- Choose the smallest native entrypoint that can advance deterministic simulation for N logical steps with fixed inputs while enabling replay writer capture (likely a new `*.cpp` tool plus Makefile target, modeled after `rng-parity-cpp`).
- Port the same extraction logic to the C# harness with identical formatting and field order.
