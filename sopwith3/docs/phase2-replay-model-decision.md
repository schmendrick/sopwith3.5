# Phase 2A Decision: Replay Model

## Purpose

Select the replay data model used for deterministic verification in Sopwith 3.5, including format, granularity, and operational workflow for both AI and human debugging.

## Decision Scope

- Single-player baseline first.
- Deterministic test workflow in C++ repository.
- Future compatibility with later C# parity checks.

## Options Considered

## Option A — Compact structured text (TSV/columnar) per frame

### Description

- Emit a stable, line-based record for selected entity fields each frame (or sampled frames).
- Example categories: planes, bombs, bullets, score/counters.

### Pros

- Human- and AI-diff friendly.
- Easy to inspect in git/CI logs.
- Lower complexity than full binary snapshots.

### Cons

- Larger than pure binary for long recordings.
- Requires strict schema/version discipline.

## Option B — Binary records + decoder

### Description

- Emit compact binary frame records and use a decoder tool for human-readable diffs/debug output.

### Pros

- Small files and low I/O overhead.
- Good for long runs.

### Cons

- Extra tooling burden (decoder required everywhere).
- Harder direct inspection during debugging.

## Option C — Hybrid (binary primary + text summaries)

### Description

- Keep binary as source artifact; generate text summaries/checkpoints for routine review.

### Pros

- Balances size and readability.

### Cons

- Highest implementation complexity among the three.

## Decision Criteria

- Deterministic reproducibility with same input+seed.
- Ease of divergence diagnosis (what changed, where, first frame).
- Implementation cost within Phase 2/3 budget.
- Compatibility with later C# parity tooling.

## Chosen Model

**Decision:** `TBD`

**Rationale:** `TBD`

## Schema Definition (to be finalized)

- Frame header fields:
  - `frame_index`
  - `seed` (or initial-seed reference)
  - `mode`
- Entity ordering rule:
  - deterministic stable order (define exact key here)
- Entity field set:
  - planes: `...`
  - bombs: `...`
  - bullets: `...`
  - global counters: `...`
- Versioning:
  - `schema_version`

## Sampling/Granularity Decision

- Capture frequency: `every frame` / `every N frames` / `hybrid`
- Full vs targeted fields: `TBD`
- Expected file size target: `TBD`

## Operational Commands (to be filled after implementation)

- Record replay:
  - `TBD`
- Playback replay (visual):
  - `TBD`
- Headless verification run:
  - `TBD`

## Acceptance Criteria

- Same replay tape + seed yields identical output across repeated runs.
- First divergence frame can be located quickly from output artifact(s).
- Format is documented in `docs/replay-usage.md`.

## Status

- Current status: **Draft**
- Owner: `TBD`
- Last updated: 2026-04-13
