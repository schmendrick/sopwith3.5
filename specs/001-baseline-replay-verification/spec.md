# Feature Specification: Baseline Replay Verification

**Feature Branch**: `001-baseline-replay-verification`  
**Created**: 2026-04-21  
**Status**: Draft  
**Input**: User description: "Create the baseline replay verification feature spec from sopwith3/docs/phase2-replay-model-decision.md. Preserve the chosen Option A model, treat schema/version rules as guidelines, row ordering contract, logical-frame cadence, and first-divergence comparison contract as normative requirements. Scope is single-player baseline first."

## Clarifications

### Session 2026-04-21

- Q: How should comparator handle schema version mismatch? → A: Always fail comparison on schema mismatch (strict hard blocker).
- Q: How should comparator handle artifacts truncated before final FRAME_END? → A: Compare up to last complete frame and emit truncation warning.
- Q: Should this feature include visual playback validation requirements? → A: Yes, include visual playback validation in this feature scope.
- Q: How should missing required row kinds be handled during comparison? → A: Fail comparison immediately when any required row kind is missing in a frame.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Produce Baseline Replay Artifacts (Priority: P1)

As a maintainer, I can generate a deterministic replay verification artifact for a single-player run
using the agreed Option A text model, so I can compare runs reliably.

**Why this priority**: Baseline artifact generation is the minimum value needed before any comparison or
parity workflow is useful.

**Independent Test**: Run the same recorded single-player tape twice with the same seed and confirm both
generated artifacts are byte-identical and contain all required row groups in each frame block.

**Acceptance Scenarios**:

1. **Given** a valid single-player replay tape and seed, **When** baseline replay artifact generation runs,
   **Then** a text artifact is produced with one `SESSION` row and ordered frame blocks.
2. **Given** a produced artifact, **When** it is inspected for required row kinds and fields,
   **Then** all required data for baseline verification is present and consistently formatted.

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

### Key Entities *(include if feature involves data)*

- **Replay Artifact**: Structured text output representing one replay verification session and its ordered frame blocks.
- **Frame Block**: The complete set of ordered rows representing one logical simulation frame.
- **Row Record**: A typed record inside a frame block (for example, session, frame-level, ground, or entity rows) with required fields.
- **Divergence Record**: Diagnostic output describing the earliest mismatch between two artifacts.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In repeated baseline runs with the same single-player tape and seed, 100% of produced artifacts are identical across at least two runs.
- **SC-002**: Comparison of two artifacts with a known injected difference reports the first divergence at the expected frame and field in 100% of validation cases.
- **SC-003**: For baseline test sessions, 100% of generated artifacts contain complete ordered frame blocks with required row groups.
- **SC-004**: Maintainers can identify mismatch location (frame, row kind, field, values) from comparison output within one review pass for all divergence test cases.
- **SC-005**: For baseline validation runs, 100% of replay samples selected for verification can be opened and viewed end-to-end in visual playback mode.

## Assumptions

- Baseline verification targets single-player runs first; multiplayer/network parity is deferred.
- Existing replay recording and playback workflow remains the source input path for this feature.
- Artifact consumers need deterministic, human-reviewable outputs suitable for automated diff workflows.
- Schema changes may evolve in later phases, but baseline comparison requires matching schema versions.
