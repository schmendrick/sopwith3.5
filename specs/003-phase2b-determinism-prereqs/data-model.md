# Data Model: Phase 2B Determinism Prerequisites

## Entity: DeterminismFixture

- **Description**: A named replay tape used to validate deterministic repeatability.
- **Required fields**:
  - `fixture_name` (`full`, `bomb`, `bird`, `computer`)
  - `tape_filename` (`full.tape`, `bomb.tape`, `bird.tape`, `computer.tape`)
  - `coverage_intent` (broad, explosion-path, flock/order-path, computer-ai-path)
- **Validation rules**:
  - Fixture set is fixed for this feature scope.
  - Each fixture must be executed in both pre-change and post-change stages.

## Entity: ValidationRun

- **Description**: One deterministic replay validation attempt for a fixture under a specific phase condition.
- **Required fields**:
  - `run_id` (unique identifier)
  - `fixture_name` (foreign key -> DeterminismFixture)
  - `phase` (`pre_change` or `post_change`)
  - `mode` (`baseline_loop` or `fixed_step`)
  - `artifact_left` (first sidecar path)
  - `artifact_right` (second sidecar path)
  - `comparison_exit_code`
  - `comparison_summary`
- **Validation rules**:
  - Repeatability comparison requires two artifacts from equivalent run conditions.
  - `phase=post_change` runs must reference the same fixture identity as baseline runs.

## Entity: RepeatabilityResult

- **Description**: Normalized outcome from comparing equivalent artifacts for one fixture and phase.
- **Required fields**:
  - `fixture_name`
  - `phase`
  - `status` (`pass` or `fail`)
  - `first_divergence_present` (boolean)
  - `notes`
- **Validation rules**:
  - Exactly one canonical result per fixture per phase/mode combination in final evidence set.
  - `status=pass` implies no divergence under the chosen contract for that run.

## Entity: DeterminismDelta

- **Description**: Before/after classification for one fixture.
- **Required fields**:
  - `fixture_name`
  - `pre_change_status`
  - `post_change_status`
  - `delta_class` (`pass_to_pass`, `fail_to_pass`, `pass_to_fail`, `fail_to_fail`)
  - `action_required` (boolean)
- **Validation rules**:
  - `pass_to_fail` always sets `action_required=true`.
  - `fail_to_pass` is flagged as a determinism improvement for release notes/evidence.

## Entity: DeterminismEvidenceSet

- **Description**: Collected fixture-level outcomes used to gate completion of Phase 2B.
- **Required fields**:
  - `feature_id` (`003-phase2b-determinism-prereqs`)
  - `generated_at`
  - `fixtures_covered_count`
  - `regression_count`
  - `improvement_count`
  - `results` (collection of RepeatabilityResult + DeterminismDelta)
- **Validation rules**:
  - `fixtures_covered_count` must equal 4 for phase-complete evidence.
  - Completion gate fails if `regression_count > 0`.

## State Transitions

1. Fixture inventory established (`DeterminismFixture`).
2. Pre-change runs executed (`ValidationRun` phase `pre_change`).
3. Pre-change outcomes recorded (`RepeatabilityResult`).
4. RNG/timer hardening implementation applied.
5. Post-change runs executed (`ValidationRun` phase `post_change`).
6. Outcome deltas computed (`DeterminismDelta`).
7. Completion evidence assembled (`DeterminismEvidenceSet`) and reviewed.
