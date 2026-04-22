# Contract: Phase 2C Gameplay Parity Surface

## Purpose

Define deterministic, byte-comparable parity requirements for **selected gameplay decision records**
and **replay writer entity ordering**, as emitted by the Phase 2C parity harnesses (C++ and C#).

This contract is **additive** relative to:

- `specs/003-phase2b-determinism-prereqs/rng-core-contract.md`
- `specs/004-rng-branch-parity/contracts/branch-parity-contract.md`

It MUST NOT redefine, remove, or alter RNG-core / RNG-branch parity row semantics.

## Tooling scope

- Applies to **parity harness tooling outputs** used for migration and maintainer verification.
- Explicitly excludes changing `sopwith3.exe` gameplay behavior as a mechanism for achieving parity.

## Inputs

Both implementations MUST accept equivalent arguments:

- `--case <case_id>`
- `--matrix-version <string>`
- `--token <replay-token>`
- `--steps <positive-int>`

The matrix file is considered part of tooling configuration (not necessarily a CLI flag in v1), but the harness MUST echo the effective `matrix_version` and `case_id` in the header.

## Header rows (fixed order)

Each run MUST print these header lines first, in this exact order:

1. `tool=<string>`
2. `matrix_version=<string>`
3. `case_id=<string>`
4. `token=<string>`
5. `normalized_token=<string>`
6. `steps=<int>`
7. `tape_ref=<logical_name>|path=<resolved_path>|sha256=<hex>`  
   - Repeat line 7 for **each** tape ref associated with the case, sorted by `logical_name` ascending byte order.
   - If a case has zero tape refs, emit exactly: `tape_ref=none`

Canonical example:

`tape_ref=short.rec|path=replays/short.rec|sha256=0123abcd...`

## Gameplay decision records (fixed ordering rules)

After headers, the harness MUST emit zero or more decision lines, each a single line formatted as:

`decision logical_step=<int> category=<category> key=<string> <stable fields...>`

### Global ordering constraints

- Decision lines MUST be sorted by:
  1. `logical_step` ascending
  2. `category` ascending (byte order of the category token)
  3. `key` ascending (byte order)
  4. stable tie-break: ascending byte order of the entire line **excluding** any `note=` suffix (see below)

- Within a single (`logical_step`, `category`, `key`), there MUST be at most one line in v1.

### Field formatting constraints (byte comparability)

- UTF-8 text output.
- Each line uses only the field separator ` ` (space) between `key=value` tokens.
- Integer fields MUST be formatted in decimal with no leading zeros (except the single digit `0`).
- Boolean fields MUST be `0` or `1`.
- No trailing spaces at EOL.

## Phase 2C categories (v1)

### `spawn_lifecycle`

Emits lifecycle transitions for tracked entity ids (implementation-defined list, but MUST be stable for a given tool version).

Minimum required `key` values:

- `entity_created`
- `entity_destroyed`

### `collision_outcome`

v1 emits **normalized transition events** derived from replay-visible state:

- `key=plane_collided` when a player/computer plane `collided` boolean changes between logical steps.
- `key=object_life` when a tracked object's `life` crosses between `>0` and `<=0` (or equivalent sentinel defined by harness).

Category strings MUST remain stable even if future versions add additional keys.

### `score_lives_delta`

Emits on logical steps where either changes:

- `key=player` fields: `score_delta`, `lives_delta` (deltas, not absolute values)
- `key=enemy` fields: same, if applicable to the scenario and tracked entities exist

### `replay_entity_order`

Emits deterministic ordering keys used by replay writer entity emission:

- `key=object_sort_keys` MUST enumerate the ordered list of deterministic sort keys used for `OBJECT` rows for that step (format stable and documented in harness help text).
- `key=entity_row_order` MUST enumerate emitted row kinds in exact emission order (`PLAYER`, `ENEMY`, `OBJECT`) as stable tokens.

## Parity success condition

For identical CLI arguments and identical matrix configuration:

- C++ stdout bytes and C# stdout bytes MUST be identical for the entire output.

## Failure reporting requirements

On mismatch, the runner MUST identify:

- `matrix_version`
- `case_id`
- first mismatch location (`logical_step`, `category`, `field`) when parsable
- first mismatch line number

Reference mismatch tuple shape (runner output, not harness stdout):

`first_mismatch matrix_version=<...> case_id=<...> logical_step=<...|n/a> category=<...|n/a> field=<...> line=<int>`

## Versioning / tape drift rules

- Any change to:
  - matrix membership,
  - case `steps`,
  - token normalization behavior used by the harness,
  - decision definitions for an existing (`category`,`key`),
  - or referenced tape contents used by a case  
  MUST bump `matrix_version` (and therefore evidence must cite the new version).

## Scope boundaries

- Validates the Phase 2C gameplay parity surface only (listed categories).
- Does not validate full simulation parity, render parity, audio parity, or network parity.
