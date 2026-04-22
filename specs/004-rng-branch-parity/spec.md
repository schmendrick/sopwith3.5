# Feature Specification: RNG Branch-Decision Parity Harness

**Feature Branch**: `004-rng-branch-parity`  
**Created**: 2026-04-22  
**Status**: Draft  
**Input**: User description: "RNG branch-decision parity harness: extend C++ and C# parity tools to validate deterministic branch outputs (troubled sound bit, explosion-type decisions) with byte-identical cross-language output and updated replay parity evidence."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Validate Branch-Decision Parity (Priority: P1)

As a maintainer, I can run parity checks that include RNG-driven branch decisions, so I can verify that
the baseline and port implementations make the same deterministic decisions, not only the same seed/stream.

**Why this priority**: Branch-level decision parity is the next required confidence layer after RNG-core parity.

**Independent Test**: Run branch-parity harness for defined token/step cases and confirm outputs are byte-identical across both implementations.

**Acceptance Scenarios**:

1. **Given** the same replay token and step count, **When** branch-parity harness runs in both implementations, **Then** decision outputs for troubled-sound-bit and explosion-type are identical.
2. **Given** a mismatch in any decision output, **When** parity comparison runs, **Then** the run fails with a clear mismatch location.

---

### User Story 2 - Keep Results Reproducible for Maintainers (Priority: P1)

As a maintainer, I can reproduce branch-parity results with one documented command, so parity checks are reliable and repeatable for local and review workflows.

**Why this priority**: Determinism verification is only useful if others can rerun it consistently.

**Independent Test**: Follow documented prerequisites and run command sequence on a clean environment to reproduce the same parity outcomes.

**Acceptance Scenarios**:

1. **Given** documented prerequisites are met, **When** maintainer runs the parity command, **Then** all configured branch-parity cases execute and report pass/fail summary.
2. **Given** a prerequisite is missing, **When** maintainer executes parity workflow, **Then** documentation clearly explains remediation steps.

---

### User Story 3 - Preserve Evidence Continuity (Priority: P2)

As a maintainer, I can extend existing parity evidence with branch-decision results, so project history shows incremental deterministic coverage growth.

**Why this priority**: Evidence continuity helps reviewers and future port work trust parity progression.

**Independent Test**: Confirm evidence records include branch-decision case matrix, outcomes, and rationale updates.

**Acceptance Scenarios**:

1. **Given** branch-parity execution is complete, **When** evidence is reviewed, **Then** it includes explicit case list, results, and deterministic rationale.
2. **Given** prior RNG-core parity evidence exists, **When** new branch-parity evidence is added, **Then** it is clearly marked as an extension rather than replacement.

---

### Edge Cases

- What happens when core RNG parity passes but branch-decision parity fails for only one decision type?
- How should parity output handle extreme step counts that still complete within expected verification runtime?
- What happens when token normalization yields equivalent tape targets from different token spellings?
- How should evidence classify mixed outcomes when some branch decisions pass and others fail?

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST extend existing parity harness outputs to include branch decisions for troubled-sound-bit and explosion-type.
- **FR-002**: System MUST run branch-parity checks using shared token and step inputs across both implementations.
- **FR-003**: System MUST fail parity execution when any branch-decision output differs between implementations.
- **FR-004**: System MUST preserve deterministic, machine-diffable output formatting for branch-parity runs.
- **FR-005**: System MUST document prerequisites and exact reproduction commands for branch-parity validation.
- **FR-006**: System MUST update feature evidence with branch-decision parity outcomes and explicit pass/fail summary.
- **FR-007**: System MUST keep this phase scoped to branch-level deterministic parity and not expand into full gameplay parity.

### Key Entities *(include if feature involves data)*

- **BranchParityCase**: One token/step test case with expected branch-decision outputs.
- **BranchDecisionOutput**: Deterministic output fields for troubled-sound-bit and explosion-type decisions at each step.
- **BranchParityResult**: Pass/fail outcome for one case, including mismatch location details when failing.
- **ParityEvidenceRecord**: Consolidated results and rationale entry for branch-parity verification.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of configured branch-parity cases execute with a definitive pass/fail result in one run.
- **SC-002**: For passing runs, branch-decision outputs are byte-identical across implementations for all tested cases.
- **SC-003**: On mismatch, maintainers can identify failing token/step/decision in one review pass from output.
- **SC-004**: Documentation allows a maintainer to reproduce branch-parity validation without additional tribal knowledge.

## Assumptions

- Existing RNG-core parity harness remains the baseline and is extended, not replaced.
- Current parity workflow command structure can accommodate additional deterministic output fields.
- Maintainers continue using established project toolchain prerequisites for deterministic verification.
- Full gameplay parity is deferred; this feature only extends deterministic branch-level coverage.
