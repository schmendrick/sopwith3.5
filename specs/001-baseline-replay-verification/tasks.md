# Tasks: Baseline Replay Verification

**Input**: Design documents from `/specs/001-baseline-replay-verification/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Include deterministic verification and playback validation tests because the specification defines measurable validation outcomes and strict comparison behavior.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Establish baseline replay verification file layout and build/test entry points.

- [X] T001 Create replay verification module skeleton in `sopwith3/src/replay_verify.h` and `sopwith3/src/replay_verify.cpp`
- [X] T002 [P] Add comparator module skeleton in `sopwith3/src/replay_compare.h` and `sopwith3/src/replay_compare.cpp`
- [X] T003 [P] Add smoke test harness script for replay verification workflow in `sopwith3/scripts/replay/verify-baseline.ps1`
- [X] T004 Add build integration for new replay verification modules in `sopwith3/src/Makefile.msys2`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Implement shared contracts and parser/comparator primitives required by all user stories.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete.

- [X] T005 Define canonical row-kind enum and shared constants in `sopwith3/src/replay_contract.h`
- [X] T006 Implement artifact row parser for `row_kind|key=value` format in `sopwith3/src/replay_parser.cpp`
- [X] T007 [P] Implement schema/session validation helpers in `sopwith3/src/replay_validate.cpp`
- [X] T008 [P] Implement deterministic ordering validator for frame blocks in `sopwith3/src/replay_ordering.cpp`
- [X] T009 Implement comparator result model (success, warning, failure, first divergence) in `sopwith3/src/replay_compare_result.h`
- [X] T010 Add foundational unit tests for parser and ordering helpers in `sopwith3/tests/replay/test_replay_parser.cpp`

**Checkpoint**: Foundation complete - user story implementation can now proceed.

---

## Phase 3: User Story 1 - Produce Baseline Replay Artifacts (Priority: P1) 🎯 MVP

**Goal**: Emit canonical baseline replay artifact files using Option A text format and required row/field contracts.

**Independent Test**: Record a replay, dump artifacts on two runs with same input/seed, and verify byte-identical outputs with complete frame blocks.

### Tests for User Story 1

- [X] T011 [P] [US1] Add artifact generation regression test for required row presence in `sopwith3/tests/replay/test_artifact_generation.cpp`
- [X] T012 [P] [US1] Add deterministic repeat-run equality test in `sopwith3/tests/replay/test_artifact_repeatability.cpp`

### Implementation for User Story 1

- [X] T013 [US1] Implement SESSION row emission with required fields in `sopwith3/src/replay_writer.cpp`
- [X] T014 [US1] Implement frame block writer (`FRAME_BEGIN`..`FRAME_END`) in `sopwith3/src/replay_writer.cpp`
- [X] T015 [US1] Implement PLAYER/ENEMY/OBJECT row serialization with stable field order in `sopwith3/src/replay_writer_entities.cpp`
- [X] T016 [US1] Wire replay artifact dump path into replay execution flow in `sopwith3/src/sopwith.cpp`
- [X] T017 [US1] Add deterministic entity ordering before emission in `sopwith3/src/replay_writer_entities.cpp`
- [X] T018 [US1] Update replay artifact usage documentation for baseline generation in `sopwith3/docs/replay-usage.md`

**Checkpoint**: User Story 1 produces deterministic baseline artifacts and passes repeatability checks.

---

## Phase 4: User Story 2 - Detect First Divergence Deterministically (Priority: P2)

**Goal**: Compare two artifacts under strict structure/version contracts and emit first divergence.

**Independent Test**: Compare known-mismatch artifact pairs and verify earliest mismatch output; verify strict failures for schema mismatch and missing required row kinds.

### Tests for User Story 2

- [X] T019 [P] [US2] Add first-divergence locator test in `sopwith3/tests/replay/test_first_divergence.cpp`
- [X] T020 [P] [US2] Add schema mismatch hard-fail test in `sopwith3/tests/replay/test_schema_mismatch.cpp`
- [X] T021 [P] [US2] Add missing required row-kind hard-fail test in `sopwith3/tests/replay/test_missing_required_row.cpp`
- [X] T022 [P] [US2] Add truncation policy test (compare complete prior frames + warning) in `sopwith3/tests/replay/test_truncation_policy.cpp`

### Implementation for User Story 2

- [X] T023 [US2] Implement comparison entrypoint and frame iteration in `sopwith3/src/replay_compare.cpp`
- [X] T024 [US2] Implement schema mismatch hard-fail branch in `sopwith3/src/replay_compare.cpp`
- [X] T025 [US2] Implement required-row and required-field failure checks in `sopwith3/src/replay_compare_structure.cpp`
- [X] T026 [US2] Implement truncation detection and warning result path in `sopwith3/src/replay_compare_structure.cpp`
- [X] T027 [US2] Implement first-divergence capture payload (`frame_index`,`row_kind`,`entity_id`,`field_name`,`lhs_value`,`rhs_value`) in `sopwith3/src/replay_compare_diff.cpp`
- [X] T028 [US2] Add comparator CLI/report output formatting in `sopwith3/src/replay_compare_cli.cpp`

**Checkpoint**: User Story 2 yields deterministic comparator outcomes with strict contract enforcement and first-divergence diagnostics.

---

## Phase 5: User Story 3 - Keep Validation Scope Explicit and Practical (Priority: P3)

**Goal**: Enforce single-player-first baseline scope and explicit contract boundaries in runtime checks and docs.

**Independent Test**: Validate comparator and artifact workflows reject unsupported scope assumptions while preserving documented baseline behavior.

### Tests for User Story 3

- [X] T029 [P] [US3] Add scope guard test for single-player baseline assumptions in `sopwith3/tests/replay/test_scope_baseline_singleplayer.cpp`

### Implementation for User Story 3

- [X] T030 [US3] Add scope guard and explicit baseline mode checks in `sopwith3/src/replay_verify.cpp`
- [X] T031 [US3] Document in-scope vs deferred scope constraints in `sopwith3/docs/replay-usage.md`
- [X] T032 [US3] Add contract terminology normalization updates in `specs/001-baseline-replay-verification/contracts/replay-verification-contract.md`

**Checkpoint**: User Story 3 documents and enforces baseline boundaries without expanding to deferred parity scope.

---

## Phase 6: User Story 4 - Validate Visual Replay Playback (Priority: P3)

**Goal**: Ensure replay artifacts used in deterministic verification are also valid for end-to-end visual inspection playback.

**Independent Test**: Run saved replay in visual mode and verify start/progress/completion behavior for baseline validation samples.

### Tests for User Story 4

- [X] T033 [P] [US4] Add visual playback smoke automation script in `sopwith3/scripts/replay/test-visual-playback.ps1`
- [X] T034 [P] [US4] Add visual replay acceptance checklist in `sopwith3/tests/replay/test_visual_playback_checklist.md`

### Implementation for User Story 4

- [X] T035 [US4] Add replay playback validation logging hooks in `sopwith3/src/sopwith.cpp`
- [X] T036 [US4] Implement replay-flow interruption detection/reporting for visual mode in `sopwith3/src/replay_visual_validation.cpp`
- [X] T037 [US4] Document visual playback validation procedure in `sopwith3/docs/replay-usage.md`

**Checkpoint**: User Story 4 enables repeatable visual replay validation for artifacts in the baseline flow.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Final consistency checks and documentation alignment across stories.

- [X] T038 [P] Run full quickstart validation sequence and capture evidence in `specs/001-baseline-replay-verification/quickstart.md`
- [X] T039 Consolidate replay verification command examples in `sopwith3/docs/replay-usage.md`
- [X] T040 [P] Perform code cleanup and shared helper refactor in `sopwith3/src/replay_compare.cpp`
- [X] T041 Verify contract/spec/plan alignment notes in `specs/001-baseline-replay-verification/research.md`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies; start immediately.
- **Foundational (Phase 2)**: Depends on Phase 1; blocks all user stories.
- **User Stories (Phases 3-6)**: Depend on Phase 2 completion.
  - Preferred order: US1 (P1) -> US2 (P2) -> US3/US4 (P3).
  - US3 and US4 can run in parallel after US2 if team capacity allows.
- **Polish (Phase 7)**: Depends on completion of selected user stories.

### User Story Dependencies

- **US1 (P1)**: Starts after Foundational; no dependency on other stories.
- **US2 (P2)**: Depends on US1 artifact production outputs and foundational comparator primitives.
- **US3 (P3)**: Depends on US1/US2 contract behavior to enforce documented boundaries.
- **US4 (P3)**: Depends on US1 replay generation and baseline playback stability; can proceed alongside US3.

### Within Each User Story

- Write story tests first and confirm failure before implementation where feasible.
- Implement data/serialization structures before comparator/reporting paths.
- Update docs/usage commands before closing story checkpoint.

### Parallel Opportunities

- Phase 1 tasks marked `[P]` can run concurrently.
- Phase 2 parser/validator helper tasks marked `[P]` can run concurrently.
- In US1, repeatability test and row-presence test can run in parallel.
- In US2, schema/row/truncation tests can run in parallel.
- US3 and US4 phases can run in parallel after US2 checkpoint.

---

## Parallel Example: User Story 2

```bash
# Launch comparator behavior tests in parallel:
Task: "T019 [US2] Add first-divergence locator test in sopwith3/tests/replay/test_first_divergence.cpp"
Task: "T020 [US2] Add schema mismatch hard-fail test in sopwith3/tests/replay/test_schema_mismatch.cpp"
Task: "T021 [US2] Add missing required row-kind hard-fail test in sopwith3/tests/replay/test_missing_required_row.cpp"
Task: "T022 [US2] Add truncation policy test in sopwith3/tests/replay/test_truncation_policy.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 (Setup).
2. Complete Phase 2 (Foundational).
3. Complete Phase 3 (US1 artifact generation).
4. Validate deterministic repeatability for US1.
5. Pause for review before comparator expansion.

### Incremental Delivery

1. Deliver US1 for stable artifact generation baseline.
2. Add US2 for strict comparator + first divergence.
3. Add US3 for explicit scope guardrails.
4. Add US4 for visual playback validation.
5. Finish with polish and evidence capture.

### Parallel Team Strategy

With multiple contributors after Phase 2:

1. Contributor A: US1/US2 comparator core path.
2. Contributor B: US3 scope/documentation hardening.
3. Contributor C: US4 visual playback validation.

---

## Notes

- All tasks follow required checklist format: `- [ ] T### [P?] [US#?] Description with file path`.
- Story-labeled tasks appear only in user story phases.
- Suggested MVP scope: **US1 only**.
- Keep deterministic and structural contract enforcement strict per clarified requirements.
