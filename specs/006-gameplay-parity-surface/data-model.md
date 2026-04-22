# Data Model: Phase 2C Gameplay Parity Surface

## ParityCaseMatrix

Represents the full set of deterministic parity scenarios executed by the gameplay parity runner.

- **matrix_version**: string, monotonic identifier (example: `2026-04-22`, `006.3`); must change when any case definition or referenced tape identity changes.
- **matrix_digest**: optional string; stable hash of the normalized matrix file contents (recommended for evidence).

### Rules

- Adding/removing/reordering cases requires a **new** `matrix_version`.
- Renaming a `case_id` is treated as a breaking matrix change (new version).

## ParityCase

One runnable scenario in the matrix.

- **case_id**: stable string identifier (immutable across matrix versions; new semantics require a new id).
- **token**: replay token string accepted by both harnesses (same semantics as RNG parity tokens today).
- **steps**: positive integer number of logical steps/frames to simulate for the case.
- **tape_refs**: list of `TapeRef` entries referenced by the case (may be empty if token resolves without extra artifacts, but must be explicit when tapes are part of the fixture).

## TapeRef

Identifies an input tape artifact used by a case.

- **logical_name**: short stable name (example: `short.rec`, `short.tape`).
- **resolved_path**: string; normalized path string produced by the same replay path normalization rules used elsewhere in parity tooling.
- **content_id**: string; deterministic content fingerprint (recommended: `sha256:` prefix) so tape edits are visible in evidence without silent drift.

## HarnessRunHeader

Emitted at the top of each harness stdout artifact (both languages must match byte-for-byte).

- **tool**: constant string identifying harness name and semver-ish tool version (example: `gameplay-parity-harness 0.1.0`).
- **matrix_version**: string
- **case_id**: string
- **token**: string
- **normalized_token**: string
- **steps**: integer
- **tape_refs**: repeated `key=value` fields in stable order (see contract)

## GameplayDecisionRecord

A single normalized gameplay decision emission within a case run.

- **logical_step**: integer step index (0-based) aligned to the harness-defined logical step boundary.
- **category**: enum string (one of the Phase 2C categories):
  - `spawn_lifecycle`
  - `collision_outcome`
  - `score_lives_delta`
  - `replay_entity_order`
- **key**: string; stable sub-key within category (example: `player_score`, `enemy_sort`, `plane_collided_transition`).
- **fields**: ordered list of `key=value` pairs **with a stable field order per (`category`,`key`)** (byte comparison depends on this).

## ParityComparisonMismatch

The diagnostic object produced by the runner on failure.

- **matrix_version**: string
- **case_id**: string
- **logical_step**: string (`n/a` allowed for header mismatches)
- **category**: string (`n/a` allowed)
- **field**: string (first differing field name; `line_count` allowed)
- **line**: 1-based line index in stdout text

## Implementation note: `content_id` / `sha256`

Harnesses emit `sha256=<64 lowercase hex chars>` for each resolved tape file. C++ computes SHA-256 in-process; C# uses `SHA256.HashData` and `Convert.ToHexString(...).ToLowerInvariant()` for the same digest semantics on Windows.
