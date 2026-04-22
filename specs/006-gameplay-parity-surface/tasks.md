# Tasks: Phase 2C Gameplay Parity Surface

**Input**: Design documents from `specs/006-gameplay-parity-surface/`  
**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`, `quickstart.md`

**Tests**: Omitted (not requested in `spec.md`; verification is harness byte-parity + manual evidence per `quickstart.md`).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no blocking dependency)
- **[USn]**: User story label (`spec.md`: US1 P1, US2 P2, US3 P3)
- Every task includes at least one concrete file path

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Versioned matrix artifact, evidence scaffold, C# project shell.

- [X] T001 Add versioned gameplay parity case matrix at `tools/gameplay-parity-matrix.json` (`matrix_version`, `cases[]` with `case_id`, `token`, `steps`, `tape_refs[]` per `specs/006-gameplay-parity-surface/data-model.md`)
- [X] T002 Create evidence scaffold at `specs/006-gameplay-parity-surface/evidence.md` (sections: matrix version, command line, pass/fail table, mismatch appendix placeholder)
- [X] T003 [P] Add `tools/csharp/GameplayParityHarness/GameplayParityHarness.csproj` targeting `net10.0` by mirroring `tools/csharp/RngParityHarness.csproj` settings

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Both harness entrypoints build, emit contract headers byte-identically, before any gameplay decision logic.

**CRITICAL**: No user story completion until T004-T007 pass (both sides print identical header block for a fixed case).

- [X] T004 Add `gameplay-parity-cpp` target to `sopwith3/src/Makefile.msys2` emitting `sopwith3/gameplay-parity-cpp.exe` (model object list after `rng-parity-cpp` pattern; start with `gameplay_parity_tool.o` + `replay_paths.o` only if sufficient for headers)
- [X] T005 Implement CLI and header-only output in `sopwith3/tools/gameplay_parity_tool.cpp` per `specs/006-gameplay-parity-surface/contracts/gameplay-parity-contract.md` (`--case`, `--matrix-version`, `--token`, `--steps`; emit `tool=` through `tape_ref=` lines; `tape_ref=none` when matrix case has no tapes)
- [X] T006 [P] Implement matching CLI and header-only output in `tools/csharp/GameplayParityHarness/Program.cs` (reuse token normalization approach from `tools/csharp/Program.cs` for `normalized=` consistency)
- [X] T007 Wire `GameplayParityHarness` build into repo workflow by documenting `dotnet build` path in `specs/006-gameplay-parity-surface/quickstart.md` (update placeholder commands to real project path once `Program.cs` exists)

**Checkpoint**: From repo root, manual run of C++ and C# binaries for one matrix case produces byte-identical stdout through header block.

---

## Phase 3: User Story 1 - Detect deterministic gameplay decision drift (Priority: P1) MVP

**Goal**: Byte-comparable gameplay `decision` lines across C++ and C# for all contract categories on the fixed matrix.

**Independent Test**: Run `tools/run-gameplay-parity.ps1` (once added); all cases `[PASS]`; inject deliberate one-field drift in either harness and confirm runner prints `first_mismatch` with `matrix_version`, `case_id`, `logical_step`, `category`, `field`, `line`.

- [X] T008 [US1] Extend `sopwith3/tools/gameplay_parity_tool.cpp` to advance a deterministic logical-step loop for `--steps` and emit sorted `decision` lines for `spawn_lifecycle` (`entity_created`, `entity_destroyed` keys) per `specs/006-gameplay-parity-surface/contracts/gameplay-parity-contract.md`
- [X] T009 [US1] Extend `sopwith3/tools/gameplay_parity_tool.cpp` to emit `collision_outcome` decisions (`plane_collided`, `object_life` keys) with stable field order when transitions occur
- [X] T010 [US1] Extend `sopwith3/tools/gameplay_parity_tool.cpp` to emit `score_lives_delta` decisions (`player`, `enemy` keys) with `score_delta` and `lives_delta` fields on change steps only
- [X] T011 [US1] Extend `sopwith3/tools/gameplay_parity_tool.cpp` to emit `replay_entity_order` decisions (`object_sort_keys`, `entity_row_order` keys) consistent with ordering rules in `sopwith3/src/replay_writer_entities.cpp`
- [X] T012 [US1] Expand `sopwith3/src/Makefile.msys2` `gameplay-parity-cpp` link set to include all object files required by `gameplay_parity_tool.cpp` after T008-T011 (no new sources into `rng_parity_tool.cpp`)
- [X] T013 [P] [US1] Mirror T008-T011 semantics and byte-for-byte formatting in `tools/csharp/GameplayParityHarness/Program.cs`
- [X] T014 [US1] Add `tools/run-gameplay-parity.ps1` modeled on `tools/run-rng-parity.ps1` to load cases from `tools/gameplay-parity-matrix.json`, invoke `sopwith3/gameplay-parity-cpp.exe` and `dotnet .../GameplayParityHarness.dll`, compare stdout bytes, and print `first_mismatch matrix_version=... case_id=... logical_step=... category=... field=... line=...` on divergence per `specs/006-gameplay-parity-surface/contracts/gameplay-parity-contract.md`
- [X] T015 [US1] Implement `tape_ref` `sha256=` computation in both `sopwith3/tools/gameplay_parity_tool.cpp` and `tools/csharp/GameplayParityHarness/Program.cs` for each resolved tape path (stable lowercase hex; same file read rules on Win11)

**Checkpoint**: US1 satisfied when full matrix passes under `tools/run-gameplay-parity.ps1` and mismatch diagnostics match spec FR-004.

---

## Phase 4: User Story 2 - Preserve Phase 2B/004 contract boundaries (Priority: P2)

**Goal**: RNG core and branch parity artifacts and contracts remain byte- and behavior-identical.

**Independent Test**: `powershell -ExecutionPolicy Bypass -File tools/run-rng-parity.ps1` exits 0 with unchanged `[PASS]` lines; `sopwith3/tools/rng_parity_tool.cpp` row format unchanged.

- [X] T016 [US2] Confirm no functional edits in `sopwith3/tools/rng_parity_tool.cpp` or `tools/csharp/Program.cs` beyond whitespace-irrelevant churn; if shared code extraction is needed, refactor into a new header only when `rng-parity-cpp` output is provably unchanged
- [X] T017 [US2] Record RNG regression command and result in `specs/006-gameplay-parity-surface/evidence.md` under a "Phase 2B/004 non-regression" subsection
- [X] T018 [US2] Add explicit "RNG parity unchanged" note to `tools/README.md` pointing to `tools/run-rng-parity.ps1` and `specs/004-rng-branch-parity/contracts/branch-parity-contract.md`

**Checkpoint**: US2 satisfied when evidence documents RNG pass and code review confirms RNG harness outputs untouched.

---

## Phase 5: User Story 3 - Reproduce and audit parity evidence (Priority: P3)

**Goal**: Reviewers can reproduce runs from docs; evidence captures matrix version, tape ids, and mismatch details.

**Independent Test**: A second maintainer follows `specs/006-gameplay-parity-surface/quickstart.md` only and reproduces the same pass/fail summary recorded in `specs/006-gameplay-parity-surface/evidence.md`.

- [X] T019 [US3] Finalize runbook in `specs/006-gameplay-parity-surface/quickstart.md` (exact `run-gameplay-parity.ps1` flags, build skip option, matrix path, expected final success line)
- [X] T020 [US3] Populate `specs/006-gameplay-parity-surface/evidence.md` with latest `matrix_version`, date, full command line, per-case PASS/FAIL, and attach first mismatch tuple when applicable
- [X] T021 [P] [US3] Optionally add a short rollup pointer in `specs/003-phase2b-determinism-prereqs/evidence.md` linking to `specs/006-gameplay-parity-surface/evidence.md` (one paragraph + link only)

**Checkpoint**: US3 satisfied when `quickstart.md` and `evidence.md` match a real successful local run.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Maintainer discoverability and contract drift guardrails.

- [X] T022 [P] Extend `tools/README.md` with a "Gameplay parity (Phase 2C)" section listing `run-gameplay-parity.ps1`, `gameplay-parity-matrix.json`, and `specs/006-gameplay-parity-surface/contracts/gameplay-parity-contract.md`
- [X] T023 Add `sha256` verification note for tape files to `specs/006-gameplay-parity-surface/data-model.md` if implementation choices diverge from initial `TapeRef` sketch (keep contract and data model aligned in same PR)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1** -> **Phase 2** -> **Phase 3 (US1)** -> **Phase 4 (US2)** / **Phase 5 (US3)** can overlap after US1 complete (US2/US3 mostly docs + regression; T016-T018 can start after T014 exists)
- **Phase 6** after gameplay runner exists (depends on T014)

### User Story Dependencies

- **US1**: Depends on Phase 2; blocks meaningful US3 evidence
- **US2**: Depends on T014 not breaking build scripts; best after US1 harness stable
- **US3**: Depends on US1 runner + populated commands

### Parallel Opportunities

- **T003** while authoring `gameplay-parity-matrix.json` (**T001**)
- **T006** alongside **T005** (different languages; align in Phase 2 checkpoint)
- **T013** after C++ decision semantics settled (**T008-T012** sequence on C++ side can lead before C# mirror)
- **T021** in parallel with **T020** once evidence structure stable

---

## Parallel Example: User Story 1 (after T008-T012 complete)

```text
Task: "Mirror collision/score/spawn/order emission in tools/csharp/GameplayParityHarness/Program.cs"
Task: "Implement tape_ref sha256= in tools/csharp/GameplayParityHarness/Program.cs"
```

---

## Implementation Strategy

### MVP First (User Story 1 only)

1. Complete Phase 1 and Phase 2 (headers byte-identical).
2. Complete Phase 3 through **T015** (full matrix green).
3. Stop and validate mismatch diagnostics with a deliberate one-line drift.

### Incremental Delivery

1. Add US2 regression notes to `specs/006-gameplay-parity-surface/evidence.md` immediately after US1 goes green.
2. Fill US3 documentation (`quickstart.md`, optional Phase 2B evidence pointer).

### Parallel Team Strategy

- Developer A: `sopwith3/tools/gameplay_parity_tool.cpp` + `sopwith3/src/Makefile.msys2`
- Developer B: `tools/csharp/GameplayParityHarness/` + C# mirror
- Developer C: `tools/run-gameplay-parity.ps1` + `tools/gameplay-parity-matrix.json`

---

## Notes

- Total tasks: **23** (T001-T023)
- Per story: **US1** = T008-T015 (8 tasks), **US2** = T016-T018 (3 tasks), **US3** = T019-T021 (3 tasks)
- MVP scope: Phase 1 + Phase 2 + Phase 3 (T001-T015)
- [P] tasks: T003, T006, T013, T021, T022 (parallel when dependencies allow)
