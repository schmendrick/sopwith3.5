# Tasks: RNG Branch-Decision Parity Harness

**Input**: Design documents from `/specs/004-rng-branch-parity/`  
**Prerequisites**: `plan.md` (required), `spec.md` (required), `research.md`, `data-model.md`, `contracts/`

**Tests**: Required. This feature is parity-validation-first; every story is validated through deterministic comparison runs.

**Organization**: Tasks are grouped by user story so each story is independently testable and deliverable.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no blocking dependency)
- **[Story]**: User story label (`[US1]`, `[US2]`, `[US3]`)
- Include exact file paths

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Prepare feature docs and harness baselines for branch-parity extension.

- [X] T001 Create branch-parity evidence file scaffold in `specs/004-rng-branch-parity/evidence.md`
- [X] T002 Align parity-harness readme references to new branch-parity feature in `parity-harness/README.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Establish shared output contract and parser assumptions used by all user stories.

**⚠️ CRITICAL**: No story implementation starts until this phase is complete.

- [X] T003 Define canonical branch-output field order in `specs/004-rng-branch-parity/contracts/branch-parity-contract.md`
- [X] T004 Implement/refresh common output parser helper for parity rows in `parity-harness/run-rng-parity.ps1`
- [X] T005 Add mismatch report format contract (`token`, `steps`, `step`, `field`) in `specs/004-rng-branch-parity/contracts/branch-parity-contract.md`

**Checkpoint**: Shared branch-parity output contract is fixed.

---

## Phase 3: User Story 1 - Validate Branch-Decision Parity (Priority: P1) 🎯 MVP

**Goal**: Extend both C++ and C# parity harnesses to emit and compare troubled-sound-bit + explosion-type decisions.

**Independent Test**: Run parity script on one token/step case and confirm byte-identical output including new branch fields.

### Tests for User Story 1

- [X] T006 [P] [US1] Add C++ branch-output regression fixture test case notes in `specs/004-rng-branch-parity/quickstart.md`
- [X] T007 [P] [US1] Add C# branch-output regression fixture test case notes in `specs/004-rng-branch-parity/quickstart.md`

### Implementation for User Story 1

- [X] T008 [US1] Extend C++ harness output rows with `troubled_sound_bit` and `explosion_type` in `sopwith3/src/rng_parity_tool.cpp`
- [X] T009 [US1] Extend C# harness output rows with `troubled_sound_bit` and `explosion_type` in `parity-harness/csharp/Program.cs`
- [X] T010 [US1] Ensure C++ build target remains valid after output extension in `sopwith3/src/Makefile.msys2`
- [X] T011 [US1] Update parity runner row-compare assumptions for new fields in `parity-harness/run-rng-parity.ps1`

**Checkpoint**: Branch-decision parity for troubled-sound-bit and explosion-type is executable and comparable.

---

## Phase 4: User Story 2 - Keep Results Reproducible (Priority: P1)

**Goal**: Guarantee one-command reproducibility with explicit prerequisites and clear failure guidance.

**Independent Test**: Follow docs from a fresh shell and run parity script successfully.

### Tests for User Story 2

- [X] T012 [P] [US2] Validate default matrix run command and expected pass output text in `parity-harness/README.md`
- [X] T013 [P] [US2] Validate expanded matrix command (`1024` steps) in `specs/004-rng-branch-parity/quickstart.md`

### Implementation for User Story 2

- [X] T014 [US2] Add explicit branch-parity reproduce commands and prerequisites in `specs/004-rng-branch-parity/quickstart.md`
- [X] T015 [US2] Add .NET/MSYS2 troubleshooting notes specific to branch parity in `parity-harness/README.md`
- [X] T016 [US2] Add deterministic command examples for token/step overrides in `parity-harness/run-rng-parity.ps1` usage section/comments

**Checkpoint**: Maintainers can reliably reproduce branch-parity checks end-to-end.

---

## Phase 5: User Story 3 - Preserve Evidence Continuity (Priority: P2)

**Goal**: Record branch-parity outcomes as an explicit extension of existing RNG-core evidence.

**Independent Test**: Evidence file clearly separates core parity and branch parity sections with case matrix and rationale.

### Tests for User Story 3

- [X] T017 [P] [US3] Define evidence acceptance checklist in `specs/004-rng-branch-parity/evidence.md`

### Implementation for User Story 3

- [X] T018 [US3] Record branch-parity case matrix and outcomes in `specs/004-rng-branch-parity/evidence.md`
- [X] T019 [US3] Record mismatch location format (or explicit all-pass note) in `specs/004-rng-branch-parity/evidence.md`
- [X] T020 [US3] Add rationale and continuity note linking to prior RNG-core evidence in `specs/004-rng-branch-parity/evidence.md`

**Checkpoint**: Branch-parity evidence is complete and traceable.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Final consistency checks across tools, docs, and evidence.

- [X] T021 [P] Re-run full parity matrix (including `1024` steps) and capture output in `specs/004-rng-branch-parity/evidence.md`
- [X] T022 Verify branch-parity contract/spec/plan/tasks alignment in `specs/004-rng-branch-parity/`
- [X] T023 [P] Update any cross-feature references from `003` to `004` where branch parity is now authoritative in `parity-harness/README.md` and `specs/004-rng-branch-parity/quickstart.md`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: starts immediately.
- **Phase 2 (Foundational)**: depends on Phase 1 and blocks all stories.
- **Phase 3 (US1)**: starts after Phase 2; establishes MVP.
- **Phase 4 (US2)**: depends on US1 output schema being in place.
- **Phase 5 (US3)**: depends on US1/US2 execution outputs and docs.
- **Phase 6 (Polish)**: final validation after selected stories complete.

### User Story Dependencies

- **US1 (P1)**: no dependency on other stories after foundation.
- **US2 (P1)**: depends on US1 extended outputs.
- **US3 (P2)**: depends on US1/US2 completed runs and documentation.

### Within Each User Story

- Add/validate story tests/check criteria first.
- Implement tool outputs before runner comparisons.
- Update docs/evidence after implementation behavior is stable.

### Parallel Opportunities

- `[P]` tasks in setup/tests/polish can run concurrently.
- US1 C++ and C# extension tasks can be split, then unified at runner task.
- Documentation updates in US2 can proceed in parallel with evidence checklist prep in US3.

---

## Parallel Example: User Story 1

```bash
# Independent prep checks in parallel
Task: "T006 [US1] quickstart C++ branch-output notes"
Task: "T007 [US1] quickstart C# branch-output notes"

# Implementation split across languages in parallel
Task: "T008 [US1] extend C++ rng_parity_tool.cpp outputs"
Task: "T009 [US1] extend C# Program.cs outputs"
```

---

## Implementation Strategy

### MVP First (US1 only)

1. Complete Setup + Foundational phases.
2. Deliver US1 branch-output parity extension.
3. Validate with one token/step case before expanding scope.

### Incremental Delivery

1. US1: branch-output parity fields + comparison.
2. US2: reproducibility and toolchain-quality docs.
3. US3: evidence continuity and reporting quality.
4. Polish: expanded matrix and final alignment checks.

### Team Parallel Strategy

With multiple contributors after foundation:

1. Contributor A: C++ harness + Makefile updates.
2. Contributor B: C# harness + runner compare logic.
3. Contributor C: docs/evidence updates and reproducibility hardening.

---

## Notes

- All tasks follow required checklist format with IDs, story labels, and file paths.
- Tests are included because parity validation is explicit in the feature scope.
- Recommended MVP scope: **US1 only**, then expand to US2/US3.
