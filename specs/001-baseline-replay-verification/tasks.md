# Tasks: Baseline Replay Verification

**Input**: Design documents from `/specs/001-baseline-replay-verification/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Include deterministic verification and playback validation tests because the specification defines measurable validation outcomes and strict comparison behavior.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

**Phase naming**: Phases **1–9** are delivered on the branch; **Phase 10** implements **FR-014–FR-016** (canonical **`*.tape`**, numbered **`*.sidecar`**, basename **`replay-compare`**).
In `spec.md`, **Phase A / B / C** is product maturity: **tasks Phase 8 = spec Phase B**, **tasks Phase 9 = spec Phase C**.

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

**Status**: **Phase A** (minimal `SESSION` sidecar + wiring) is complete. **Phase B** (full `SESSION` + per-frame blocks at logical cadence) is tracked in Phase 8 below.

**Independent Test (current)**: Record twice with the same tape/seed; **`SESSION`** rows match across emitted sidecars (Phase A; filenames follow **Phase 10** once implemented).

**Independent Test (Phase B onward)**: Byte-identical full artifacts including complete ordered frame blocks.

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

**Note**: T013–T017 cover writer modules and hooks; **emitting a full per-frame sidecar during record** is
closed under **Phase 8** (T043–T045), not this checkpoint.

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

## Phase 8: Phase B — Full `SESSION` + per-frame Option A emission *(follow-up)*

**Purpose**: Satisfy `spec.md` Phase B: complete session identity and logical-frame dumps into sidecar files (see **Phase 10** for **`basename.n.sidecar`** naming); enables SC-001/SC-003 for full artifacts.

**Prerequisites**: Phase 3 scaffold (sidecar path, writer modules, tests harness) complete.

- [X] T042 [US1] Extend `SESSION` with decision-doc fields (`gamemode`, `session_id`, `version`, and other CLI/runtime identity needed for comparable baselines) in `sopwith3/src/replay_writer.cpp` and wire sources from `sopwith3/src/sopwith.cpp`
- [X] T043 [US1] Drive `FRAME_BEGIN`…`FRAME_END` emission once per logical simulation frame during record/replay dump in `sopwith3/src/sopwith.cpp`, using existing helpers in `sopwith3/src/replay_writer.cpp`
- [X] T044 [US1] Ensure PLAYER/ENEMY/OBJECT (and any other required kinds) serialize with stable ordering each frame in `sopwith3/src/replay_writer_entities.cpp`
- [X] T045 [P] [US1] Update `sopwith3/tests/replay/test_artifact_generation.cpp` and `sopwith3/tests/replay/test_artifact_repeatability.cpp` to assert full frame contract and byte-identical full sidecars (Phase B acceptance)
- [X] T046 [P] [US1] Refresh `sopwith3/docs/replay-usage.md` and `specs/001-baseline-replay-verification/quickstart.md` for full-artifact workflow

**Checkpoint**: Two runs with identical inputs produce byte-identical full sidecar artifacts including all required frame blocks.

---

## Phase 9: Phase C — Comparator on real emitted artifacts *(follow-up)*

**Purpose**: Close the loop from emitted tapes to strict compare + first divergence using production frame data, not fixture-only paths.

**Prerequisites**: Phase 8 checkpoint (real per-frame emission).

- [X] T047 [US2] Add integration path: compare two sidecar text files produced by the game in `sopwith3/scripts/replay/verify-baseline.ps1` (or sibling script), failing on schema/row/field mismatch per contract
- [X] T048 [P] [US2] Add regression fixture update process note in `specs/001-baseline-replay-verification/contracts/replay-verification-contract.md` once live emission stabilizes

**Checkpoint**: Baseline verification script exercises real dumps end-to-end with documented expected outcomes.

---

## Phase 10: Filesystem contract — `*.tape`, `*.n.sidecar`, basename `replay-compare` *(FR-014–FR-016)*

**Purpose**: Implement normative paths from `spec.md` (2026-04): canonical binary **`basename.tape`** after token normalization; verification sidecars **`tape-basename.<n>.sidecar`** with **`max(n)+1`** allocation; **`replay-compare <basename>`** discovers cwd matches (exactly **two** ⇒ compare; **0/1/>2** ⇒ list / error per FR-016).

**Prerequisites**: Phases 1–9 complete (comparator + writer plumbing exists).

**Independent test**: `-vshort` and `-vshort.rec` open the same **`short.tape`**; successive runs emit **`short.1.sidecar`**, **`short.2.sidecar`**; **`replay-compare.exe short`** succeeds only with exactly two matches and byte-equal content.

### Implementation for User Story 1 (paths & sidecars)

- [X] T049 [US1] Add replay path helpers (`normalize_replay_token_to_tape_path`, `next_sidecar_index`, `make_sidecar_path`) in `sopwith3/src/replay_paths.cpp` and `sopwith3/src/replay_paths.h` per **Tape path normalization** and **Sidecar naming** in `spec.md`
- [X] T050 [US1] Open binary record/playback streams using normalized **`.tape`** paths only in `sopwith3/src/sopwith.cpp` (`inithistory` / file open sites for `recordfilename` / `playbackfilename`)
- [X] T051 [US1] Allocate and open verification writer to **`basename.(max+1).sidecar`** (scan `basename.*.sidecar` in tape directory) replacing **`stateTape + ".state.txt"`** wiring in `sopwith3/src/sopwith.cpp`
- [X] T052 [US1] Ensure `session.session_id` / `replay_tape_basename` track normalized tape stem consistently after path changes in `sopwith3/src/sopwith.cpp`

### Implementation for User Story 2 (comparator CLI)

- [X] T053 [US2] Extend `main` / usage in `sopwith3/tools/replay_compare_tool.cpp`: **two args** unchanged; **one arg** basename ⇒ glob cwd **`basename.<n>.sidecar`**, numeric sort, print matched paths; **exactly two** ⇒ **`replay_compare_files`**; otherwise non-zero per FR-016
- [X] T054 [P] [US2] Register new replay tests target sources in `sopwith3/src/Makefile.msys2` if adding new test translation units

### Tests *(spec mandates measurable outcomes)*

- [X] T055 [P] [US1] Add unit tests for tape normalization (`short`, `short.rec`, `short.tape`, relative paths) in `sopwith3/tests/replay/test_replay_paths.cpp`
- [X] T056 [P] [US2] Add integration-style tests for basename **`replay-compare`** exit codes (0/1/2/>2 matches) in `sopwith3/scripts/replay/run-tests.ps1` (basename smoke: **>2** matches ⇒ exit 1)

### Cross-cutting docs & tooling

- [X] T057 [US3] Update `-h`/`-v` helptext strings that still mention **`.state.txt`** to **`basename.n.sidecar`** / **`.tape`** in `sopwith3/src/sopwith.cpp`
- [X] T058 [P] Align `sopwith3/scripts/replay/verify-baseline.ps1` parameter docs and examples with **`*.sidecar`** filenames
- [X] T059 [P] Add ignore patterns for **`*.sidecar`** (and canonical **`*.tape`** if needed) in `.gitignore` under `sopwith3/` or repo root per team preference
- [X] T060 Run `specs/001-baseline-replay-verification/quickstart.md` validation sequence after Phase 10 code lands and refresh evidence notes in that file


**Checkpoint**: FR-014–FR-016 satisfied in code; SC-006 / SC-007 testable via automated or scripted runs.

### Parallel Example: Phase 10

```bash
# After T049–T053 land, tests and tooling can proceed in parallel:
Task: "T055 [US1] test_replay_paths.cpp"
Task: "T056 [US2] run-tests.ps1 basename smoke"
Task: "T058 verify-baseline.ps1"
Task: "T059 .gitignore patterns"
```

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies; start immediately.
- **Foundational (Phase 2)**: Depends on Phase 1; blocks all user stories.
- **User Stories (Phases 3-6)**: Depend on Phase 2 completion.
  - Preferred order: US1 (P1) -> US2 (P2) -> US3/US4 (P3).
  - US3 and US4 can run in parallel after US2 if team capacity allows.
- **Polish (Phase 7)**: Depends on completion of selected user stories.
- **Phase B (Phase 8)**: Depends on Phase 3 sidecar + writer plumbing; replaces “SESSION-only” acceptance with full artifact acceptance.
- **Phase C (Phase 9)**: Depends on Phase 8; validates comparator + scripts against live emission.
- **Phase 10 (Filesystem contract)**: Depends on Phases 8–9 (full emission + compare plumbing); touches **`sopwith.cpp`**, new **`replay_paths.*`**, **`replay_compare_tool.cpp`**, tests, scripts.

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
3. Complete Phase 3 (US1 artifact generation — **Phase A** `SESSION` scaffold on this branch).
4. Validate deterministic repeatability for US1 (Phase A scope).
5. Pause for review before comparator expansion.

### Incremental Delivery

1. Deliver US1 for stable artifact generation baseline (Phase A done; **Phase 8** completes spec Phase B).
2. Add US2 for strict comparator + first divergence.
3. Add US3 for explicit scope guardrails.
4. Add US4 for visual playback validation.
5. Finish with polish and evidence capture.
6. **Phase 8–9**: full sidecar emission + compare real dumps (see checkpoints in those sections).
7. **Phase 10**: canonical **`*.tape`**, numbered **`*.sidecar`**, basename **`replay-compare`** (FR-014–FR-016).

### Parallel Team Strategy

With multiple contributors after Phase 2:

1. Contributor A: US1/US2 comparator core path.
2. Contributor B: US3 scope/documentation hardening.
3. Contributor C: US4 visual playback validation.

---

## Notes

- All tasks follow required checklist format: `- [ ] T### [P?] [US#?] Description with file path`.
- Story-labeled tasks appear only in user story phases.
- Delivered MVP for this branch: **Phase A** (minimal `SESSION` + harness + visual validation). **Phase 8–9** carry Phase B/C from `spec.md`. **Phase 10** implements filesystem naming (**FR-014–FR-016**).
- Keep deterministic and structural contract enforcement strict per clarified requirements.
