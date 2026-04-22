# Data Model: Baseline Replay Verification

## Physical storage (filesystem)

These are naming/path rules for **binary replay tapes** and **verification sidecars** on disk. They do not change row payloads inside artifacts.

### ReplayTapeFile

- **Description**: Canonical binary history file opened for record (**`-h`**) or playback (**`-v`**).
- **Path rule**: Directory prefix from replay token + normalized basename **`strip_repeated(.tape|.rec) + ".tape"`** (see **spec.md**).
- **Examples**: Token `demo.rec` in cwd → **`demo.tape`**; token `subdir\x.run.tape` → **`subdir\x.run.tape`** after stripping.

### SidecarFile

- **Description**: UTF-8 text artifact containing structured replay verification rows (`SESSION`, frame blocks).
- **Path rule**: **`{tape_dir}/{tape_stem}.{n}.sidecar`** where **`tape_stem`** is basename of **`ReplayTapeFile`** without the **`.tape`** suffix, and **`n`** is a positive decimal integer assigned at emission time.
- **Allocation rule**: **`n = max(S) + 1`** where **S** is the set of integers parsed from existing **`{tape_stem}.*.sidecar`** names in **`tape_dir`**; if **S** empty, **`n = 1`**.

### ComparatorInvocation *(tooling)*

- **Two-path mode**: **`replay-compare <left.sidecar> <right.sidecar>`** — applies **Comparison Contract** once.
- **Basename mode**: **`replay-compare <basename>`** — discovers **`basename.*.sidecar`** in cwd, sorts by numeric suffix. **Exactly two** files ⇒ **Comparison Contract** once. **Fewer than two** or **more than two** ⇒ no compare (listing / error paths per **spec.md**).

---

## Entity: ReplaySession

- **Description**: Metadata row that identifies one replay verification artifact.
- **Required fields**:
  - `schema_version` (integer, required)
  - `session_id` (string, required)
  - `initial_seed` (integer, required)
  - `gamemode` (enum/string, required)
  - `version` (string/integer, required)
  - `latency` (integer, required)
  - `playerindex` (integer, required)
- **Validation rules**:
  - Exactly one `SESSION` row per artifact
  - Must appear before all frame rows

## Entity: FrameBlock

- **Description**: One logical simulation frame represented by bounded ordered rows.
- **Identity**:
  - `frame_index` (integer, unique per artifact)
- **Required row sequence**:
  1. `FRAME_BEGIN`
  2. `FRAME`
  3. `GROUND`
  4. zero or more `PLAYER` rows (single-player baseline expects one)
  5. zero or more `ENEMY` rows
  6. zero or more `OBJECT` rows
  7. `FRAME_END`
- **Validation rules**:
  - Missing required row kind => comparator failure
  - Out-of-order rows => comparator failure
  - Truncated final frame without `FRAME_END` => compare only completed prior frames and emit warning

## Entity: FrameGlobalsRow (`FRAME`)

- **Description**: Frame-level deterministic state required for parity.
- **Required fields**:
  - `frame_index`
  - `randv`
  - `latencycount`
  - `framecounter`
  - `object_count`
  - `input_source`
  - `input_mask_player_raw`
  - `input_mask_player_effective`

## Entity: GroundRow (`GROUND`)

- **Description**: Deterministic terrain snapshot for one frame.
- **Required fields**:
  - `frame_index`
  - `ground_count`
  - `ground_values`
- **Validation rules**:
  - `ground_count` must equal configured horizontal range size
  - `ground_values` length must match `ground_count`

## Entity: ActorRow (`PLAYER`, `ENEMY`)

- **Description**: Plane-state snapshot rows used for parity checks.
- **Identity**:
  - `entity_id` (monotonic within run, never reused)
- **Required fields**:
  - `entity_id`, `state`, `endstatus`
  - position/velocity split fields (`x_int`, `x_frac`, `y_int`, `y_frac`, `xv_int`, `xv_frac`, `yv_int`, `yv_frac`)
  - `angle`, `speed`, `accel`, `life`
  - plus player-only fields in baseline (`lives`, `ammo`, `bombs`, `score`, `flags`)
- **Ordering rule**:
  - Sorted ascending by `entity_id` within each row kind

## Entity: ObjectRow (`OBJECT`)

- **Description**: Non-plane simulation objects included in deterministic comparison.
- **Identity**:
  - `entity_id` (monotonic within run)
- **Required fields**:
  - `entity_id`, `object_kind`, `owner_entity_id`, `state`
  - position/velocity split fields
  - `life` (nullable by object type semantics)
- **Ordering rule**:
  - Sorted ascending by `entity_id`

## Entity: DivergenceRecord

- **Description**: First mismatch emitted by comparator.
- **Required fields**:
  - `frame_index`
  - `row_kind`
  - `field_name`
  - `lhs_value`
  - `rhs_value`
  - `entity_id` (when applicable)
- **Lifecycle**:
  - Created once per **two-artifact** comparison run at earliest mismatch.
  - No additional mismatch records required in baseline mode

## State Transitions

- **Artifact lifecycle**:
  1. Replay input accepted
  2. Session row emitted
  3. Frame blocks emitted in increasing `frame_index`
  4. Artifact finalized
  5. Comparator validates structure
  6. Comparator either reports success, warning+success (truncated tail policy), or first divergence/failure
