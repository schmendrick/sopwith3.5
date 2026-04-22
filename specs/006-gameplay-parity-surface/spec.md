# Feature Specification: Phase 2C Gameplay Parity Surface

**Feature Branch**: `006-gameplay-parity-surface`  
**Created**: 2026-04-22  
**Status**: Draft  
**Input**: User description: "Phase 2C gameplay parity surface: extend deterministic C++/C# parity coverage beyond RNG branch outputs to selected gameplay event decisions and ordering (spawn/lifecycle, collision outcome categories, score/lives deltas, and replay-writer entity ordering) using a fixed token/step case matrix, byte-comparable cross-language output, explicit first-mismatch diagnostics, and updated reproducibility/evidence docs while preserving current Phase 2B/004 contracts and scope boundaries."

## Clarifications

### Session 2026-04-22

- Q: Should byte comparison require a cross-platform canonical text format or raw bytes as emitted? → A: Raw emitted bytes compared only within the designated Win11 maintainer environment.
- Q: How should the fixed token/step case matrix evolve when input tapes change or the matrix grows? → A: Versioned matrix with explicit bumps; stable case identifiers; tape references captured for reproducibility.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Detect deterministic gameplay decision drift (Priority: P1)

As a maintainer, I can run a fixed parity case matrix and verify that selected gameplay decision outcomes and ordering are byte-comparable across both language implementations so regressions are caught immediately.

**Why this priority**: Deterministic parity drift in gameplay decisions directly threatens replay trustworthiness and must be surfaced first.

**Independent Test**: Execute the defined case matrix in both implementations and confirm outputs are byte-comparable for all selected decision categories.

**Acceptance Scenarios**:

1. **Given** the approved fixed token/step case matrix, **When** both implementations run all cases, **Then** outputs match byte-for-byte for all covered gameplay decision categories.
2. **Given** an intentional parity divergence in one decision field, **When** comparison runs, **Then** the first mismatch is reported with exact case identifier, step index, category, and differing values.

---

### User Story 2 - Preserve existing Phase 2B/004 contract boundaries (Priority: P2)

As a maintainer, I can extend parity coverage without changing existing Phase 2B/004 deterministic contracts, enabling safe incremental verification.

**Why this priority**: Scope discipline prevents accidental contract churn and keeps already validated parity surfaces stable.

**Independent Test**: Re-run existing Phase 2B/004 parity checks and verify prior contract outputs remain unchanged while new gameplay parity categories are added.

**Acceptance Scenarios**:

1. **Given** the prior deterministic contract suite, **When** Phase 2C parity extensions are applied, **Then** all existing contract outputs and pass/fail behavior remain unchanged.
2. **Given** new gameplay decision categories, **When** parity artifacts are produced, **Then** they appear as additive coverage and do not alter previously defined fields or semantics.

---

### User Story 3 - Reproduce and audit parity evidence (Priority: P3)

As a reviewer, I can follow updated reproducibility and evidence documentation to rerun the same matrix version and independently confirm parity outcomes.

**Why this priority**: Evidence must remain reproducible so parity claims are auditable and repeatable by other contributors.

**Independent Test**: Follow the documented replay/parity evidence steps from a clean environment and confirm identical pass/fail outcome and evidence structure.

**Acceptance Scenarios**:

1. **Given** the updated reproducibility instructions, **When** a reviewer executes them, **Then** they can produce the expected evidence artifacts for the documented matrix version without undocumented steps.
2. **Given** a run with a known mismatch, **When** evidence is generated, **Then** diagnostic reporting includes the first mismatch details and enough context to reproduce the failure.

---

### Edge Cases

- A case includes no spawn/lifecycle transitions for a category; parity output still emits a deterministic empty representation rather than omitting the category.
- Multiple decision categories diverge in the same run; reporting still identifies only the earliest mismatch by deterministic ordering rules.
- Collision outcomes produce equivalent high-level categories through different internal paths; parity comparison evaluates the normalized category contract, not internal path details.
- Score/lives deltas net to zero across a step sequence; parity output still captures the deltas deterministically for comparison.
- Replay-writer entity ordering inputs contain ties; ordering output follows stable deterministic tie-breaking so byte comparison remains reliable.
- The parity case matrix grows (new cases) or underlying input tapes change; the matrix version increments and evidence records which matrix version and tape identities were used.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST extend deterministic parity coverage from RNG branch outputs to the selected gameplay decision surface: spawn/lifecycle decisions, collision outcome categories, score/lives deltas, and replay-writer entity ordering.
- **FR-002**: The system MUST execute parity checks using a versioned token/step case matrix with stable per-case identifiers so each case is reproducible across runs and matrix evolution is auditable.
- **FR-003**: The system MUST produce byte-comparable parity output across both language implementations for every covered case and decision category when run in the designated Win11 maintainer environment.
- **FR-004**: The system MUST report the first mismatch only, including at minimum case identifier, deterministic step index, decision category, and expected/actual values.
- **FR-005**: The system MUST preserve all existing Phase 2B/004 deterministic contract fields and semantics without modification.
- **FR-006**: The system MUST treat the new gameplay parity fields as additive scope and MUST NOT expand into unrelated gameplay parity domains outside the listed categories.
- **FR-007**: The system MUST produce evidence artifacts that clearly show pass/fail status for each fixed case and expose first-mismatch diagnostics when failures occur.
- **FR-008**: The system MUST provide reproducibility documentation that allows another maintainer to run the same fixed matrix version and verify matching parity outcomes.
- **FR-009**: The system MUST define deterministic ordering rules for emitted gameplay decision records so parity comparison is stable when multiple events occur within the same step window.
- **FR-010**: The system MUST confine changes to parity tooling, parity comparison artifacts, and related documentation, and MUST NOT change `sopwith3.exe` runtime gameplay behavior as part of this feature.
- **FR-011**: The system MUST record the matrix version and the identities of any referenced input tapes used by each matrix entry so tape changes do not silently invalidate historical comparisons.

### Key Entities *(include if feature involves data)*

- **Parity Case Matrix**: A versioned collection of deterministic parity scenarios; changes to membership, steps, tokens, or referenced tapes require a new matrix version.
- **Parity Case Matrix Entry**: A deterministic test case definition containing token set, step boundaries, a stable case identifier, and references to any input tapes used by the case.
- **Gameplay Decision Record**: A normalized parity record for one covered decision category at a specific deterministic step, including category-specific outcome values.
- **Parity Comparison Result**: A run outcome containing pass/fail state, first-mismatch details (if any), and per-case completion status.
- **Evidence Package**: The reproducibility-facing artifact set that captures matrix version, run context, and parity results for audit and replay validation.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of cases in a documented matrix version complete with byte-comparable outputs across both implementations for all covered gameplay decision categories in a clean parity run.
- **SC-002**: In seeded divergence tests, the first mismatch location and category are reported within a single comparison run for 100% of failing cases.
- **SC-003**: Existing Phase 2B/004 parity checks retain a 100% pass rate after Phase 2C changes, demonstrating no regression to prior contract surfaces.
- **SC-004**: At least one independent maintainer can reproduce documented parity evidence end-to-end for the same matrix version and referenced tape identities, reaching the same pass/fail outcome without ad hoc instructions.

## Assumptions

- The existing deterministic token generation and step progression inputs remain authoritative and available for building the fixed matrix.
- The parity case matrix is expected to grow over time; the count of input tapes is not a permanent invariant, but matrix versioning makes changes explicit.
- The two parity implementations continue to expose equivalent logical gameplay decision information needed for normalization.
- Phase 2C scope is limited to the explicitly listed decision categories and does not include full gameplay simulation parity.
- Existing evidence artifact conventions remain the baseline format, with additive updates for new gameplay parity fields.
- Any gameplay-runtime adjustments to `sopwith3.exe` are handled by separate features and are out of scope for this specification.
- Byte-level parity validation is required only for the designated Win11 maintainer environment and not as a cross-platform compatibility guarantee.
