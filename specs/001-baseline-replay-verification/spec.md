# Feature Specification: Baseline Replay Verification

**Feature Branch**: `001-baseline-replay-verification`  
**Created**: 2026-04-21  
**Updated**: 2026-04-24 (feature close-out: delivery phases aligned with completed `tasks.md`; status implemented)  
**Status**: Implemented  
**Input**: User description: "Create the baseline replay verification feature spec from sopwith3/docs/phase2-replay-model-decision.md. Preserve the chosen Option A model; row ordering contract, logical-frame cadence, and first-divergence comparison contract as normative requirements; schema/version match as a strict comparison gate (see clarifications). Scope is single-player baseline first." Subsequent updates (2026-04-22): tape filename normalization (`*.tape`), numbered sidecar files (`*.n.sidecar`), single-basename `replay-compare` discovery (exactly two sidecars to compare), CLI record/playback flag wording aligned with current `sopwith3/src` behavior.

## CLI record vs playback *(terminology)*

Examples and requirements use **record** and **playback** semantics. Behavior applies to whichever CLI flags currently implement recording vs playback in **`sopwith3/src`** (today: **`-h<replay>` records** binary tape; **`-v<replay>` plays back** binary tape—see options parsing there). Documentation and examples MUST stay consistent with that mapping even if historical Sopwith documentation used different letters.

## Delivery phases *(implementation alignment)*

Shipped milestones align with [`tasks.md`](tasks.md) Phases 3–10:

- **Phase A (historical scaffold)**: Early wiring with minimal `SESSION`, harness, and instrumentation. Superseded by full emission for current builds.
- **Phase B — delivered** (`tasks.md` Phase 8): Complete `SESSION` per decision doc (including `gamemode`, `session_id`, `version`, and fields needed for comparable baselines). Emits ordered per-frame blocks (`FRAME_BEGIN` … `FRAME_END`) at logical simulation cadence with required row kinds/fields.
- **Phase C — delivered** (`tasks.md` Phase 9): Comparator and verification scripts run end-to-end against **live** emitted sidecars (integration path and documented fixture/process notes).
- **Filesystem contract — delivered** (`tasks.md` Phase 10; FR-014–FR-016): Canonical **`basename.tape`** after replay-token normalization; numbered **`basename.<n>.sidecar`** with **`max(existing n)+1`** allocation; **`replay-compare <basename>`** discovery (exactly two matches ⇒ one compare; list-only + non-zero when not exactly two).

Normative success criteria in this spec apply to the **shipped** baseline (full frame blocks + filesystem contract). Legacy Phase-A-only artifacts are not the acceptance target for new work.

## Clarifications

### Session 2026-04-21

- Q: How should comparator handle schema version mismatch? → A: Always fail comparison on schema mismatch (strict hard blocker).
- Q: How should comparator handle artifacts truncated before final FRAME_END? → A: Compare up to last complete frame and emit truncation warning.
- Q: Should this feature include visual playback validation requirements? → A: Yes, include visual playback validation in this feature scope.
- Q: How should missing required row kinds be handled during comparison? → A: Fail comparison immediately when any required row kind is missing in a frame.

### Session 2026-04-22

- Tape I/O MUST use a single canonical binary filename **`<normalized-basename>.tape`** derived from the replay token passed with record/playback flags (normalization rules below).
- Sidecar text artifacts MUST use **`basename.<positive-n>.sidecar`** with monotonic numbering per basename (collision rules below).
- Single-basename **`replay-compare`** discovers matching sidecars in the working directory; **exactly two** matches run one two-file compare; **more than two** matches list files only (no comparison sweep).

### Session 2026-04-23

- Q: When two OS processes run **`sopwith3.exe`** concurrently and both allocate a new sidecar for the **same** basename in the **same** directory, what must the implementation guarantee? → A: **Out of scope for v1**: assume a **single writer** at a time; concurrent overlapping runs have **undefined** collision behavior for **`max(n)+1`** allocation.
- Q: Should single-basename **`replay-compare`** run exhaustive pairwise compares when **three or more** sidecars exist? → A: **No** — compare **only** when **exactly two** sidecars match; if **more than two** match, **print** the discovered paths (which files were considered) and exit **non-zero** without running comparisons; use **two-argument** mode to compare a chosen pair.

## Tape path normalization *(normative)*

- **Input**: The replay token is the string attached to **`-h`** or **`-v`** with no space (for example `-vshort`, `-hmy_run`, `-vsubdir\demo`).
- **Binary tape path**: Split path into directory prefix (if any) and final path component (**basename**). Normalize **basename only**:
  1. Repeatedly strip a trailing extension if it equals, case-insensitively, **`.tape`** or **`.rec`** (strip at most one recognized suffix per loop until basename no longer ends with either).
  2. Append **`.tape`** to the stripped basename.
  3. Recombine with the original directory prefix unchanged.
- **Read/write**: All binary replay file opens for record or playback MUST use this normalized path. The engine does not read or write ambiguous `short` vs `short.rec` vs `short.tape` as distinct tapes; the normalized `short.tape` wins.
- **Examples**:
  - Token `short` → `short.tape`
  - Token `short.tape` → `short.tape`
  - Token `C:\runs\demo.rec` → `C:\runs\demo.tape`
- **Example command line** (PowerShell, pass-through): `.\sopwith3.exe --% -vshort -s -i` performs single-player playback with IBM keyboard layout; replay token `short` normalizes to binary tape **`short.tape`** in the working directory.

## Sidecar naming and collision handling *(normative)*

- **Pattern**: `<tape-basename>.<n>.sidecar` where `<tape-basename>` is the **basename without `.tape`** of the normalized binary tape path, and `<n>` is a **positive decimal integer** (1, 2, 3, …).
- **First emission**: For basename `short`, first sidecar file is **`short.1.sidecar`**.
- **Further runs**: Before creating a new sidecar, scan the directory containing the tape for files matching `<tape-basename>.*.sidecar` where `*` parses as a positive integer. Let **N** be the **maximum** such integer across **all** matching names (not merely “first gap”). The next emission uses **`<tape-basename>.(N+1).sidecar`**. If no matches exist, use **`1`**.
- **“Exists”**: Any filename `basename.n.sidecar` with valid integer **n** counts; gaps (e.g. missing `short.2.sidecar` while `short.3.sidecar` exists) do not reset numbering—next file is **`max(n)+1`**.
- **Concurrency (v1)**: Allocation assumes **at most one process** at a time creates new sidecars for a given basename in that directory; concurrent processes racing on **`max(n)+1`** are **undefined** (see Clarifications Session 2026-04-23).
- Implementation and documentation MUST replace prior **`<replay>.state.txt`** wording with this scheme.

## Single-basename `replay-compare` *(normative)*

- **Invocation**: `replay-compare.exe <basename>` — exactly **one** argument, **no** extension; `<basename>` matches sidecars such as `short.1.sidecar`, `short.2.sidecar`.
- **Discovery**: In the **current working directory**, collect every file named `<basename>.<n>.sidecar` where **n** parses as a positive integer.
- **Sort order**: Order discovered files by **numeric n ascending** (not lexicographic string sort of filenames).
- **Stdout**: MUST list which sidecar files were matched (**full filenames or paths**, stable order matching numeric sort). A single line or multiple lines is acceptable as long as the full set is unambiguous.
- **Exactly two matches**: Run the **existing two-file** first-divergence comparison on those two paths; exit code follows that comparison (**zero** only on full success under the contract).
- **Zero or one match**: Exit **non-zero** with a clear message that **exactly two** sidecar files are required for automatic comparison (or use **two-argument** mode with explicit paths).
- **More than two matches**: **Do not** run comparisons. Still **print** the discovered sidecar paths so the operator sees which files were found; exit **non-zero** with a clear message that automatic compare applies only when **exactly two** matches exist — pick two paths and invoke **`replay-compare <left> <right>`** explicitly.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Produce Baseline Replay Artifacts (Priority: P1)

As a maintainer, I can generate a deterministic replay verification artifact for a single-player run
using the agreed Option A text model, so I can compare runs reliably.

**Why this priority**: Baseline artifact generation is the minimum value needed before any comparison or
parity workflow is useful.

**Independent Test (minimal)**: Record or play twice under the same single-player tape and seed; confirm the **`SESSION`** line is byte-identical across the two emitted **`basename.<n>.sidecar`** files (indices may differ between runs—content must still match under controlled inputs).

**Independent Test (full baseline)**: Same as minimal, and confirm both artifacts are byte-identical including all required row groups in each frame block.

**Acceptance Scenarios**:

1. **Given** a valid single-player replay tape and seed, **When** baseline replay artifact generation runs,
   **Then** a text artifact is produced with one `SESSION` row and ordered frame blocks per logical frame.
2. **Given** a produced artifact, **When** it is inspected for required row kinds and fields,
   **Then** all required data for baseline verification is present and consistently formatted under the full frame contract.
3. **Given** a replay token passed with `-h` or `-v`, **When** the engine resolves binary tape paths,
   **Then** normalization yields a single canonical `.tape` path and sidecars follow `<basename>.<n>.sidecar` rules.

---

### User Story 2 - Detect First Divergence Deterministically (Priority: P2)

As a maintainer, I can compare two baseline replay artifacts and get the first divergence record, so I
can diagnose regressions quickly.

**Why this priority**: Comparison and first-divergence reporting are core to the regression workflow.

**Independent Test**: Compare two intentionally different artifacts and verify the process reports the
earliest mismatching frame/row/field with both values.

**Acceptance Scenarios**:

1. **Given** two baseline artifacts with a mismatch, **When** comparison runs, **Then** it stops at the
   first mismatch and reports frame, row kind, field, and compared values.
2. **Given** two identical baseline artifacts, **When** comparison runs, **Then** it reports no divergence.
3. **Given** three or more sidecars for one basename, **When** single-basename **`replay-compare`** runs,
   **Then** it lists the matched files and exits non-zero **without** comparing (maintainer uses two-arg mode for a chosen pair).

---

### User Story 3 - Keep Validation Scope Explicit and Practical (Priority: P3)

As a maintainer, I can rely on a clearly bounded single-player baseline contract, so verification is
stable before broader scenarios are added.

**Why this priority**: Explicit v1 scope avoids unstable requirements and supports incremental delivery.

**Independent Test**: Review the spec and confirm it defines single-player-first scope and distinguishes
normative contracts from versioning guidelines.

**Acceptance Scenarios**:

1. **Given** the baseline replay specification, **When** scope is reviewed, **Then** single-player-first
   constraints are explicit and testable.
2. **Given** replay evolution planning, **When** versioning behavior is reviewed, **Then** schema/version
   rules are captured as strict parity gate requirements for v1.

---

### User Story 4 - Validate Visual Replay Playback (Priority: P3)

As a maintainer, I can validate that a saved replay opens and plays visually end-to-end, so artifact
verification and human inspection workflows stay aligned.

**Why this priority**: Visual playback is a secondary but required confidence path for interpreting
verification outputs.

**Independent Test**: Run a saved baseline replay in visual mode and confirm it starts, progresses, and
completes without replay-flow failure.

**Acceptance Scenarios**:

1. **Given** a valid saved replay, **When** visual playback is launched, **Then** the replay starts and
   progresses through frames without playback-flow interruption.
2. **Given** a replay used for deterministic artifact comparison, **When** visual playback is reviewed,
   **Then** the run can be inspected end-to-end as a human diagnostic path.

---

### Edge Cases

- What happens when a replay artifact is truncated and ends before `FRAME_END` for the final frame?
- Missing required row kind in any frame causes immediate comparison failure.
- What happens when entity counts match but entity ordering differs inside a row group?
- How is comparison handled when row kinds are present but one required field is missing?
- Replay token includes only `.tape` / `.rec` variants: normalization still yields one canonical `.tape` path.
- Mixed sidecar indices exist (`basename.1.sidecar`, `basename.5.sidecar`): next emission uses **6**, not **2**.
- Single-basename **`replay-compare`** with exactly two matching sidecars: runs **one** two-file comparison.
- Single-basename mode with **more than two** matching sidecars: lists matches only; **no** automatic comparisons.
- Two concurrent **`sopwith3.exe`** runs that both create a new sidecar for the same basename: **undefined** in v1—do not rely on **`max(n)+1`** without a single writer.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST define baseline replay artifacts using the Option A compact structured text model.
- **FR-002**: System MUST represent replay output as one session section followed by ordered logical-frame blocks.
- **FR-003**: System MUST enforce a deterministic row ordering contract within each frame block for comparison readiness.
- **FR-004**: System MUST sample replay data at logical simulation frame cadence, independent of render cadence.
- **FR-005**: System MUST include required baseline row types and fields needed for single-player verification.
- **FR-006**: System MUST compare two baseline artifacts by validating contract structure before field comparisons.
- **FR-007**: System MUST stop at first mismatch and emit a first-divergence record containing frame index, row kind, field name, and left/right values.
- **FR-008**: System MUST support successful comparison outcomes when artifacts are fully equivalent under the baseline contract.
- **FR-009**: System MUST reject comparison attempts when required structural contracts (row presence, order, or required fields) are violated.
- **FR-009c**: System MUST fail comparison immediately when any required row kind is missing in a frame block.
- **FR-009a**: System MUST detect artifact truncation where the final frame block is incomplete.
- **FR-009b**: System MUST compare only complete frame blocks up to the last valid `FRAME_END` when truncation is detected, and MUST emit a truncation warning.
- **FR-010**: System MUST fail comparison when schema versions differ.
- **FR-011**: System MUST keep v1 verification scope limited to single-player baseline behavior.
- **FR-012**: System MUST include visual replay playback validation as part of baseline feature acceptance.
- **FR-013**: System MUST ensure a replay used for baseline verification can be opened for end-to-end human inspection playback.
- **FR-014**: System MUST normalize binary replay tape paths so that record and playback always read/write **`<basename>.tape`** per the tape normalization rules in this spec, regardless of whether the user included `.rec` or `.tape` in the replay token.
- **FR-015**: System MUST emit sidecar text artifacts as **`<tape-basename>.<n>.sidecar`** with integer **n ≥ 1**, choosing **n** by scanning existing **`basename.*.sidecar`** files and allocating **max(existing n)+1** (or **1** if none). **Concurrency**: v1 assumes a single allocating process per basename directory (undefined if concurrent).
- **FR-016**: **`replay-compare`** MUST support a single **basename** argument (no extension): discover **`basename.*.sidecar`** in the working directory, sort by numeric **n** ascending, and print the matched sidecar paths. If **exactly two** files match, MUST run the existing two-artifact comparison on them. If **zero or one** match, MUST exit non-zero with a clear message. If **more than two** match, MUST **not** run comparisons, MUST still print which files matched, and MUST exit non-zero with a clear message that **exactly two** sidecars are required for automatic compare (**two-argument** mode compares an explicit pair).

### Key Entities *(include if feature involves data)*

- **Replay Artifact**: Structured text output representing one replay verification session and its ordered frame blocks (stored in a **`.sidecar`** file).
- **Replay Tape**: Canonical binary input history file **`<basename>.tape`** used for record/playback after normalization.
- **Frame Block**: The complete set of ordered rows representing one logical simulation frame.
- **Row Record**: A typed record inside a frame block (for example, session, frame-level, ground, or entity rows) with required fields.
- **Divergence Record**: Diagnostic output describing the earliest mismatch between two artifacts.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In repeated baseline runs with the same single-player tape and seed, 100% of pairwise
  comparisons across runs report identical **`SESSION`** rows and identical full artifacts (including frame blocks)—independent of which **`n`** each run allocated for the sidecar filename.
- **SC-002**: Comparison of two artifacts with a known injected difference reports the first divergence at the expected frame and field in 100% of validation cases.
- **SC-003**: For baseline test sessions, 100% of generated artifacts contain complete ordered frame blocks with required row groups.
- **SC-004**: Maintainers can identify mismatch location (frame, row kind, field, values) from comparison output within one review pass for all divergence test cases.
- **SC-005**: For baseline validation runs, 100% of replay samples selected for verification can be opened and viewed end-to-end in visual playback mode.
- **SC-006**: For any replay token, normalized tape path resolves consistently: two tokens that differ only by `.rec` vs `.tape` suffix yield the same opened binary file path.
- **SC-007**: Single-basename **replay-compare** with **more than two** matching sidecars **always** lists the matched files and exits non-zero **without** invoking comparisons; with **exactly two** matches, comparison outcome matches two-argument mode (success only when artifacts are equivalent under contract).

## Assumptions

- Baseline verification targets single-player runs first; multiplayer/network parity is deferred.
- Existing replay recording and playback workflow remains the source input path for this feature, augmented by canonical `.tape` naming.
- Artifact consumers need deterministic, human-reviewable outputs suitable for automated diff workflows.
- Schema changes may evolve in later phases, but baseline comparison requires matching schema versions.
- Session identity for comparison includes mode and related CLI-affecting settings from the complete `SESSION` row; differing settings imply different baselines, not “similar” parity runs.
- Two-argument **`replay-compare`** compares explicit paths; single-basename mode (FR-016) is a convenience when **exactly two** sidecars exist for that basename.
- Maintainers run **one** recording/playback session at a time per basename output directory when relying on deterministic sidecar numbering (concurrent processes undefined for v1).

## Feature close-out

After implementation, use [`CLOSEOUT.md`](CLOSEOUT.md) for branch merge and optional tagging steps.
