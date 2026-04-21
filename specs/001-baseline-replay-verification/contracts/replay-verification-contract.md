# Contract: Replay Verification Baseline

## Purpose

Define the contract for producing and comparing baseline replay verification artifacts.

Terminology normalization: this document uses "artifact" for emitted text sidecar output and
"replay tape" for recorded input history (canonical **`*.tape`** binary file).

## File naming contract

- **Binary tape**: Normalized **`basename.tape`** per **spec.md** (replay token from **`-h`/`-v`**).
- **Verification sidecar**: **`basename.<n>.sidecar`** with positive integer **`n`**, allocation **`max(existing)+1`** in the tape directory.
- **Legacy `*.state.txt`**: Deprecated; MUST NOT appear in new conformance checks once implementation lands.

## CLI: replay-compare

- **Two arguments**: **`replay-compare <left> <right>`** — compares two artifact paths using **Comparison Contract** below.
- **One argument**: **`replay-compare <basename>`** — **basename** has no extension; discovers **`basename.<n>.sidecar`** in the **current working directory**; sorts by numeric **n** ascending; prints loaded paths on **first line of stdout**; exits non-zero if fewer than **two** matches or if **any** pairwise comparison fails; otherwise compares **all unordered pairs** **(nᵢ, nⱼ)** with **nᵢ < nⱼ** using the same contract.

## Artifact Production Contract

- Encoding: UTF-8 text
- Field format: `key=value`
- Row delimiter: one record per line
- Row type key: `row_kind`
- Session/header requirement: exactly one `SESSION` row before frame rows
- Frame boundary requirement: `FRAME_BEGIN` ... `FRAME_END`

## Required Row Types

- `SESSION`
- `FRAME_BEGIN`
- `FRAME`
- `GROUND`
- `PLAYER` (single-player baseline expects one per frame)
- `ENEMY` (0..n)
- `OBJECT` (0..n): each row MUST include **`object_kind`** (`Bomb`, `Bullet`, `Bird`, … per schema v2+) so consumers can validate fields per type.
- `FRAME_END`

## Ordering Contract

For each frame:
1. `FRAME_BEGIN`
2. `FRAME`
3. `GROUND`
4. `PLAYER*` ordered by `entity_id` ascending
5. `ENEMY*` ordered by `entity_id` ascending
6. `OBJECT*` ordered by `entity_id` ascending
7. `FRAME_END`

Across artifact:
1. `SESSION`
2. frame blocks in increasing `frame_index`

## Comparison Contract

1. Validate both artifacts contain one valid `SESSION` row.
2. Validate `schema_version` equality; mismatch => fail.
3. Validate row presence/order contract per frame.
4. Validate required field presence for each required row.
5. Compare required fields in deterministic order.
6. Stop at first mismatch and emit divergence record.

## Edge-Case Policies

- Missing required row kind in any frame => immediate fail.
- Missing required field in required row => immediate fail.
- Truncated artifact tail missing final `FRAME_END` => compare only complete prior frames and emit truncation warning.

## Divergence Record Minimum Shape

- `frame_index`
- `row_kind`
- `entity_id` (if row has entity identity)
- `field_name`
- `lhs_value`
- `rhs_value`

## Regression fixtures after live emission stabilizes

When `schema_version`, `engine_version` (`SESSION|version=`), entity field sets, or terrain serialization
(`GROUND|...`) change intentionally, update any checked-in `*.sidecar` fixtures and the replay unit tests
that embed canonical artifacts. Prefer regenerating fixtures from two identical controlled runs (same
tape, seed, and CLI flags) and committing the pair only after `replay-compare.exe` reports success.
