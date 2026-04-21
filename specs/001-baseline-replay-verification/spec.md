# Feature Specification: Baseline Replay Verification

**Feature Branch**: `001-baseline-replay-verification`  
**Created**: 2026-04-21  
**Updated**: 2026-04-22  
**Status**: Draft  
**Input**: User description: "Create the baseline replay verification feature spec from sopwith3/docs/phase2-replay-model-decision.md. Preserve the chosen Option A model; row ordering contract, logical-frame cadence, and first-divergence comparison contract as normative requirements; schema/version match as a strict comparison gate (see clarifications). Scope is single-player baseline first." Subsequent updates (2026-04-22): tape filename normalization (`*.tape`), numbered sidecar files (`*.n.sidecar`), batch `replay-compare` discovery and pairwise comparison matrix, CLI record/playback flag wording aligned with current `sopwith3/src` behavior.

## CLI record vs playback *(terminology)*

Examples and requirements use **record** and **playback** semantics. Behavior applies to whichever CLI flags currently implement recording vs playback in **`sopwith3/src`** (today: **`-h<replay>` records** binary tape; **`-v<replay>` plays back** binary tape—see options parsing there). Documentation and examples MUST stay consistent with that mapping even if historical Sopwith documentation used different letters.

## Delivery phases *(implementation alignment)*

- **Phase A (done / scaffold)**: Text sidecar next to the normalized binary tape (`<replay>.<n>.sidecar`; see naming below) with a minimal
  `SESSION` row; replay test harness and playback instrumentation logs. Serves wiring and manual smoke,
  not full parity dumps yet.
- **Phase B (next)**: Complete `SESSION` per decision doc (including `gamemode`, `session_id`, `version`,
  and any other fields required so two runs are comparable only when session identity matches). Emit
  per-frame blocks (`FRAME_BEGIN` … `FRAME_END`) at logical-frame cadence with required row kinds/fields.
- **Phase C**: Comparator consumes full artifact contract (structure + field equality + first divergence)
  end-to-end against real emitted frames.

Until Phase B is complete, success criteria that require full frame blocks apply to **Phase B onward**,
not to Phase A scaffold output.

## Clarifications

### Session 2026-04-21

- Q: How should comparator handle schema version mismatch? → A: Always fail comparison on schema mismatch (strict hard blocker).
- Q: How should comparator handle artifacts truncated before final FRAME_END? → A: Compare up to last complete frame and emit truncation warning.
- Q: Should this feature include visual playback validation requirements? → A: Yes, include visual playback validation in this feature scope.
- Q: How should missing required row kinds be handled during comparison? → A: Fail comparison immediately when any required row kind is missing in a frame.

### Session 2026-04-22

- Tape I/O MUST use a single canonical binary filename **`<normalized-basename>.tape`** derived from the replay token passed with record/playback flags (normalization rules below).
- Sidecar text artifacts MUST use **`basename.<positive-n>.sidecar`** with monotonic numbering per basename (collision rules below).
- Batch comparison CLI accepts a single replay basename, discovers matching sidecars in the working directory, and compares **all unordered pairs** of sidecar files (see matrix below).

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
  - Token `short.rec` → `short.tape`
  - Token `C:\runs\demo.rec` → `C:\runs\demo.tape`
- **Example command line** (PowerShell, pass-through): `.\sopwith3.exe --% -vshort -s -i` performs single-player playback with IBM keyboard layout; replay token `short` normalizes to binary tape **`short.tape`** in the working directory.

## Sidecar naming and collision handling *(normative)*

- **Pattern**: `<tape-basename>.<n>.sidecar` where `<tape-basename>` is the **basename without `.tape`** of the normalized binary tape path, and `<n>` is a **positive decimal integer** (1, 2, 3, …).
- **First emission**: For basename `short`, first sidecar file is **`short.1.sidecar`**.
- **Further runs**: Before creating a new sidecar, scan the directory containing the tape for files matching `<tape-basename>.*.sidecar` where `*` parses as a positive integer. Let **N** be the **maximum** such integer across **all** matching names (not merely “first gap”). The next emission uses **`<tape-basename>.(N+1).sidecar`**. If no matches exist, use **`1`**.
- **“Exists”**: Any filename `basename.n.sidecar` with valid integer **n** counts; gaps (e.g. missing `short.2.sidecar` while `short.3.sidecar` exists) do not reset numbering—next file is **`max(n)+1`**.
- Implementation and documentation MUST replace prior **`<replay>.state.txt`** wording with this scheme.

## Batch `replay-compare` *(normative)*

- **Invocation**: `replay-compare.exe <basename>` — exactly **one** argument, **no** extension; `<basename>` is the replay basename matching sidecars (for example `short` for files `short.1.sidecar`, `short.2.sidecar`).
- **Discovery**: In the **current working directory**, collect every file named `<basename>.<n>.sidecar` where **n** parses as a positive integer.
- **Sort order**: Order discovered files by **numeric n ascending** (not lexicographic string sort of filenames).
- **Stdout**: Line **1** MUST list which sidecar files were loaded (full filenames or paths, stable order matching numeric sort).
- **Minimum count**: If **zero** or **one** matching sidecar exists, the tool MUST exit **non-zero** with a clear message that batch compare requires **at least two** artifacts.
- **Comparison matrix** (when **two or more** sidecars): Run the **existing two-file** first-divergence comparison logic on **every unordered pair** of the discovered files. Order of pairs: by increasing **(nᵢ, nⱼ)** with **nᵢ < nⱼ** (equivalently: nested loops over the sorted list with inner index greater than outer).
- **Outcome**: Exit **non-zero** if **any** pairwise comparison reports divergence or structural failure; exit **zero** only if **all** pairs succeed under the existing contract.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Produce Baseline Replay Artifacts (Priority: P1)

As a maintainer, I can generate a deterministic replay verification artifact for a single-player run
using the agreed Option A text model, so I can compare runs reliably.

**Why this priority**: Baseline artifact generation is the minimum value needed before any comparison or
parity workflow is useful.

**Independent Test (Phase A)**: Record or play twice under the same single-player tape and seed; confirm the **`SESSION`** line is byte-identical across the two emitted **`basename.<n>.sidecar`** files (indices may differ between runs—content must still match under controlled inputs).

**Independent Test (Phase B onward)**: Same as Phase A, and confirm both artifacts are byte-identical
including all required row groups in each frame block.

**Acceptance Scenarios**:

1. **Given** a valid single-player replay tape and seed, **When** baseline replay artifact generation runs,
   **Then** a text artifact is produced with one `SESSION` row; **after Phase B**, it also includes ordered
   frame blocks per logical frame.
2. **Given** a produced artifact, **When** it is inspected for required row kinds and fields,
   **Then** all required data for baseline verification is present and consistently formatted (**Phase B onward**
   for full frame contract; Phase A validates `SESSION` only).
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
3. **Given** three or more sidecars for one basename, **When** batch `replay-compare` runs,
   **Then** every unordered pair is checked and failures surface with non-zero exit.

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
- Batch compare with exactly two sidecars: one pairwise comparison (the pair).

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
- **FR-015**: System MUST emit sidecar text artifacts as **`<tape-basename>.<n>.sidecar`** with integer **n ≥ 1**, choosing **n** by scanning existing **`basename.*.sidecar`** files and allocating **max(existing n)+1** (or **1** if none).
- **FR-016**: Batch **`replay-compare`** MUST accept a single basename argument, discover **`basename.*.sidecar`** in the working directory, sort by numeric **n** ascending, print loaded files on the first stdout line, and exit non-zero with a clear message when fewer than two sidecars exist.
- **FR-017**: When **two or more** sidecars are present, batch **`replay-compare`** MUST execute the existing two-artifact comparison on **every unordered pair** **(nᵢ, nⱼ)** with **nᵢ < nⱼ** in sorted order, and MUST exit non-zero if **any** pair fails.

### Key Entities *(include if feature involves data)*

- **Replay Artifact**: Structured text output representing one replay verification session and its ordered frame blocks (stored in a **`.sidecar`** file).
- **Replay Tape**: Canonical binary input history file **`<basename>.tape`** used for record/playback after normalization.
- **Frame Block**: The complete set of ordered rows representing one logical simulation frame.
- **Row Record**: A typed record inside a frame block (for example, session, frame-level, ground, or entity rows) with required fields.
- **Divergence Record**: Diagnostic output describing the earliest mismatch between two artifacts.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In repeated baseline runs with the same single-player tape and seed, 100% of pairwise
  comparisons across runs report identical **`SESSION`** rows (**Phase A**) and identical full artifacts (**Phase B onward**, including frame blocks)—independent of which **`n`** each run allocated for the sidecar filename.
- **SC-002**: Comparison of two artifacts with a known injected difference reports the first divergence at the expected frame and field in 100% of validation cases.
- **SC-003**: For baseline test sessions, 100% of generated artifacts contain complete ordered frame blocks with required row groups (**Phase B onward**; Phase A does not claim this yet).
- **SC-004**: Maintainers can identify mismatch location (frame, row kind, field, values) from comparison output within one review pass for all divergence test cases.
- **SC-005**: For baseline validation runs, 100% of replay samples selected for verification can be opened and viewed end-to-end in visual playback mode.
- **SC-006**: For any replay token, normalized tape path resolves consistently: two tokens that differ only by `.rec` vs `.tape` suffix yield the same opened binary file path.
- **SC-007**: Batch **replay-compare** with three or more sidecars completes every unordered pairwise comparison and fails the run when any pairwise comparison fails.

## Assumptions

- Baseline verification targets single-player runs first; multiplayer/network parity is deferred.
- Existing replay recording and playback workflow remains the source input path for this feature, augmented by canonical `.tape` naming.
- Artifact consumers need deterministic, human-reviewable outputs suitable for automated diff workflows.
- Schema changes may evolve in later phases, but baseline comparison requires matching schema versions.
- Session identity for comparison includes mode and related CLI-affecting settings once Phase B emits a
  complete `SESSION` row; differing settings imply different baselines, not “similar” parity runs.
- Two-argument **`replay-compare`** may remain for direct file paths; batch mode is additive per FR-016/FR-017.
