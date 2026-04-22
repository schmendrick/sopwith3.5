# Phase 2A Decision: Replay Model

## Purpose

Select the replay artifact model used for deterministic verification in Sopwith 3.5, including format, granularity, stable ordering, and divergence reporting for AI-assisted debugging and future C# parity checks.
We need a proper specification out of this, because it needs to be checked for each type found here if everything that needs to be stored got CAUGHT.

## Decision Scope

- Single-player baseline first.
- Deterministic test workflow in this C++ repository.
- Replay artifacts must be directly comparable against future C# outputs.
- This Phase 2A document is a decision/spec artifact; headless execution support itself is implemented in later phases (Phase 2B/3).

## Options Considered

## Option A — Compact structured text (TSV/columnar) per frame

### Description

- Emit line-based, diff-friendly records for required frame/global/entity fields.
- Store one logical frame snapshot per simulation step.

### Pros

- Human- and AI-diff friendly.
- Directly usable in git/CI artifacts.
- Lowest tooling overhead for first implementation.

### Cons

- Larger than pure binary for long runs.
- Requires strict schema/version discipline.

## Option B — Binary records + decoder

### Description

- Emit compact binary frame records and require a decoder for diagnostics.

### Pros

- Small files, low I/O overhead.

### Cons

- Extra tooling burden.
- Slower debugging workflow due to decode step.

## Option C — Hybrid (binary primary + text summaries)

### Description

- Persist binary as source of truth and generate text summaries/checkpoints.

### Pros

- Balances storage and readability.

### Cons

- Highest implementation complexity.
- More moving pieces before deterministic baseline is proven.

## Decision Criteria

- Deterministic reproducibility with same input + seed.
- Fast first-divergence diagnosis.
- Implementation cost within Phase 2/3 budget.
- Portability to C# parity tooling.

## Chosen Model

**Decision:** **Option A** (compact structured text per logical frame) for v1 replay verification artifacts.

**Rationale:**

- Best fit for deterministic bring-up and parity debugging.
- Minimizes tooling complexity while Phase 2B/3 are still stabilizing.
- Enables direct CI diffs and AI-readable diagnostics.
- Binary/hybrid can be added later as optimization once parity workflow is stable.

## v1 Artifact + Schema

### Format

- UTF-8 text file, `|`-separated `key=value` fields.
- One logical frame block per `frame_index`.
- `row_kind` defines row type:
  - `SESSION` (exactly one per artifact, before any frame rows)
  - `FRAME_BEGIN` (exactly one per frame)
  - `FRAME` (exactly one per frame)
  - `GROUND` (exactly one per frame)
  - `PLAYER` (exactly one per frame in single-player baseline)
  - `ENEMY` (zero or more per frame)
  - `OBJECT` (zero or more per frame)
  - `FRAME_END` (exactly one per frame)
- Frame boundaries are explicit (`FRAME_BEGIN` ... `FRAME_END`) so parsers can recover from partial/corrupt tails and still locate complete frames.
- Global cadence invariant:
  - All replay rows are sampled once per **logical simulation frame**.
  - Every row inside a single frame block (`FRAME_BEGIN` ... `FRAME_END`) represents the same simulation step.
  - Replay fields are not tied to render-frame cadence or raw OS event timing.

### Required session-level fields (`SESSION`)

- `schema_version` (initially `1`)
- `session_id`
- `initial_seed`
- `gamemode`
- `version`
- `latency`
- `playerindex`

### Required frame/global fields (`F`)

- `frame_index`
- `randv`
- `latencycount`
- `framecounter`
- `object_count`
- `input_source`
- `input_mask_player_raw`
- `input_mask_player_effective`

### Required ground/floor fields (`G`)

- `frame_index`
- `ground_count` (must be `MAX_X`)
- `ground_values` (ordered sequence for all `ground[x]`, `x=0..MAX_X-1`)
- v1 encoding rule: **raw full snapshot every frame** (no RLE, no delta-compression in v1 baseline artifacts).

### Required player-plane fields (`PLAYER`)

- `entity_id`
- `state`
- `endstatus`
- `x_int`, `x_frac`, `y_int`, `y_frac`
- `xv_int`, `xv_frac`, `yv_int`, `yv_frac`
- `angle`
- `speed`
- `accel`
- `life`
- `lives`
- `ammo`
- `bombs`
- `score`
- `flags` (`inverted`, `athome`, `goinghome` packed as deterministic bit flags)

### Required enemy-plane fields (`ENEMY`)

- `entity_id`
- `state`
- `endstatus`
- `x_int`, `x_frac`, `y_int`, `y_frac`
- `xv_int`, `xv_frac`, `yv_int`, `yv_frac`
- `angle`
- `speed`
- `accel`
- `life`
- `flags` (`inverted`, `athome`, `goinghome` packed as deterministic bit flags)

### Required non-plane object fields (`O`)

- `entity_id`
- `object_kind` (stable token, e.g. `Bomb`, `Bullet`, `Bird`, `Smoke`, `Frag`, `Ox`, `Flag`, `Chimney`, `Fuel`, `Tank`)
- `owner_entity_id` (if applicable; else empty)
- `state` (if applicable; else empty)
- `x_int`, `x_frac`, `y_int`, `y_frac`
- `xv_int`, `xv_frac`, `yv_int`, `yv_frac`
- `life` (if applicable; else empty)

### Versioning

- Schema must include `schema_version`.
- Any field addition/removal/semantic change increments schema version.
- Comparisons must reject mismatched schema versions unless an explicit converter is used.

## Input Capture Contract

Input fields follow the global logical-frame cadence invariant above.

- `input_source`
  - Type: string enum.
  - Allowed values in v1: `live`, `tape`.
  - Meaning:
    - `live`: command bits originated from runtime input devices.
    - `tape`: command bits originated from replay/history playback.

- `input_mask_player_raw`
  - Type: unsigned integer bitmask.
  - Meaning: direct command-intent mask produced for the current logical frame before final replay/timing shaping.
  - Notes:
    - Backend/runtime diagnostic field.
    - May differ across implementations due to input backend/event plumbing differences.

- `input_mask_player_effective`
  - Type: unsigned integer bitmask.
  - Meaning: final command mask actually consumed by simulation logic for the current frame.
  - Notes:
    - Must use the same command-bit definitions as gameplay (`KEY_*` semantics).
    - This is the **normative parity field** for cross-implementation comparison.
    - Replay divergence checks compare this field exactly (no tolerance).

## Deterministic Ordering and Identity Contract

- Every runtime entity participating in dumps must have a stable replay identity: `entity_id` (monotonic creation index, never reused within a run).
- Ordering inside each frame is strict:
  1. `FRAME_BEGIN` row
  2. `FRAME` row
  3. `GROUND` row
  4. all `PLAYER` rows sorted by `entity_id` ascending
  5. all `ENEMY` rows sorted by `entity_id` ascending
  6. all `OBJECT` rows sorted by `entity_id` ascending
  7. `FRAME_END` row
- Artifact-level ordering is strict:
  1. one `SESSION` row
  2. zero or more frame blocks, each following the frame ordering above
- If two implementations produce the same state but different insertion order, `entity_id` contract is the source of truth to keep artifacts comparable.
- Lifecycle rule:
  - State transitions in-place keep the same `entity_id` (example: building INTACT -> DESTROYED, `Ox` ALIVE -> DEAD).
  - Destroyed/removed and later recreated entities receive a new `entity_id` (example: flock/bird bursts, enemy respawns).

## Sampling / Granularity Decision

- Capture frequency: **every logical frame** for baseline deterministic verification.
- Field strategy: **targeted required fields** listed above (not full memory snapshots).
- Default baseline profile target:
  - 3-5 minute single-player runs should stay comfortably reviewable in text form.
  - If artifacts become too large, compression is allowed as transport/storage optimization, but canonical content remains this text schema.

## Expected Artifact Size (AI estimate)

- For a 30-second single-player replay with per-frame capture:
  - Light scene: ~0.8 MB to ~1.5 MB
  - Typical scene: ~1.5 MB to ~4 MB
  - Heavy object density (many bullets/fragments/smoke): ~4 MB to ~6+ MB
- These are planning estimates, not acceptance limits; Phase 3 should measure real distributions and revise guidance.

## Divergence Comparison Contract

- Required compared fields:
  - session-level: all required `SESSION` fields
  - frame-level: all required fields defined in this document (`FRAME_BEGIN`, `FRAME`, `GROUND`, `PLAYER`, `ENEMY`, `OBJECT`, `FRAME_END`)
- Tolerance policy (v1): **exact match only** (no epsilon tolerance, i.e. no "close enough" numeric comparisons).
- Comparator algorithm (normative):
  1. Validate identical `schema_version`.
  2. Validate `SESSION` row presence and equality of required session-level fields.
  3. Iterate frames in ascending `frame_index`.
  4. For each frame, validate row presence/order (`FRAME_BEGIN`,`FRAME`,`GROUND`,`PLAYER*`,`ENEMY*`,`OBJECT*`,`FRAME_END`).
  5. Validate `PLAYER`/`ENEMY`/`OBJECT` row counts and ordered `entity_id` sequence.
  6. Compare required fields in deterministic field order.
  7. Stop at first mismatch and emit a first-divergence record.
- First divergence is therefore the earliest frame/row/field that violates steps 3-5.
- Comparator output must include at minimum:
  - `frame_index`
  - `row_kind` (`FRAME_BEGIN`/`FRAME`/`GROUND`/`PLAYER`/`ENEMY`/`OBJECT`/`FRAME_END`)
  - `entity_id` (if applicable)
  - `field_name`
  - `lhs_value`
  - `rhs_value`

## Replay Consumption Modes

- **Headless verification replay**: deterministic validation and diff generation (primary).
- **Visual playback replay**: human inspection path (secondary but required in Phase 3).

## Operational Commands (implementation notes)

These commands are normative targets for Phase 3 documentation and may be adjusted to actual flag names once implemented.

- Record replay tape:
  - existing baseline: `sopwith3.exe -h<replay_file>`
- Playback replay visually:
  - existing baseline: `sopwith3.exe -v<replay_file>`
- Emit deterministic frame dump during replay (planned):
  - `sopwith3.exe -v<replay_file> --dump-state <state_file>`
- Headless deterministic verification mode (planned):
  - `sopwith3.exe --headless --replay <replay_file> --dump-state <state_file>`

## Acceptance Criteria

- Same replay tape + seed yields identical v1 artifact across repeated runs.
- First divergence frame is located by the defined comparison contract.
- Schema and usage are documented in `docs/replay-usage.md`.

## Example (illustrative)

```text
SESSION|schema_version=1|session_id=baseline_001|initial_seed=173|gamemode=SINGLE|version=2|latency=1|playerindex=0
....
FRAME_BEGIN|frame_index=42
FRAME|frame_index=42|randv=8123|latencycount=0|framecounter=42|object_count=7|input_source=live|input_mask_player_raw=2048|input_mask_player_effective=2048
GROUND|frame_index=42|ground_count=3000|ground_values=41,41,41,...,37
PLAYER|entity_id=1|state=FLYING|endstatus=NOTFINISHED|x_int=1270|x_frac=0|y_int=145|y_frac=0|xv_int=2|xv_frac=0|yv_int=0|yv_frac=0|angle=6|speed=3|accel=2|life=2700|lives=5|ammo=191|bombs=4|score=120|flags=athome:0,inverted:0,goinghome:0
ENEMY|entity_id=14|state=FLYING|endstatus=NOTFINISHED|x_int=1930|x_frac=0|y_int=132|y_frac=0|xv_int=-2|xv_frac=0|yv_int=0|yv_frac=0|angle=10|speed=3|accel=1|life=2550|flags=athome:0,inverted:0,goinghome:0
OBJECT|entity_id=24|object_kind=Bird|owner_entity_id=|state=|x_int=310|x_frac=0|y_int=160|y_frac=0|xv_int=1|xv_frac=0|yv_int=0|yv_frac=0|life=29
FRAME_END|frame_index=42
FRAME_BEGIN|frame_index=43
FRAME|...
GROUND|...
...
FRAME_END|frame_index=43
```

## Status

- Current status: **Under Review**
- Owner: 
- Last updated: 2026-04-14
