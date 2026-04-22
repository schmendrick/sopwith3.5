# Data Model: RNG Branch-Decision Parity Harness

## Entity: BranchParityCase

- **Description**: One test case for cross-language branch-decision parity.
- **Fields**:
  - `token` (replay token input)
  - `steps` (number of deterministic iterations)
  - `case_id` (stable identifier)
- **Validation rules**:
  - `token` must be non-empty.
  - `steps` must be positive.

## Entity: BranchDecisionStep

- **Description**: Per-step deterministic output row including RNG-core and branch outputs.
- **Fields**:
  - `step_index`
  - `randv`
  - `x`
  - `y`
  - `i`
  - `v2_type` (existing derived value)
  - `troubled_sound_bit` (branch-decision output)
  - `explosion_type` (branch-decision output)
- **Validation rules**:
  - All fields are required for byte-comparison parity.
  - Field ordering is fixed and stable across languages.

## Entity: BranchParityResult

- **Description**: Outcome of comparing one C++ and one C# run for a single case.
- **Fields**:
  - `case_id`
  - `status` (`pass` or `fail`)
  - `mismatch_step` (nullable)
  - `mismatch_field` (nullable)
  - `summary`
- **Validation rules**:
  - If `status=pass`, mismatch fields are null.
  - If `status=fail`, mismatch step/field must be populated.

## Entity: BranchParityEvidenceRecord

- **Description**: Aggregated branch-parity results published for this feature.
- **Fields**:
  - `feature_id`
  - `executed_at`
  - `total_cases`
  - `passed_cases`
  - `failed_cases`
  - `results` (collection of BranchParityResult)
  - `rationale_note`
- **Validation rules**:
  - `total_cases = passed_cases + failed_cases`.
  - Record must include reproduction command and toolchain assumptions.

## State Transitions

1. Define case matrix (`BranchParityCase`).
2. Produce C++ and C# step outputs (`BranchDecisionStep`).
3. Compare outputs and classify per-case status (`BranchParityResult`).
4. Aggregate and publish branch evidence (`BranchParityEvidenceRecord`).
