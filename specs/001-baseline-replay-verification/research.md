# Research: Baseline Replay Verification

## Decision 1: Artifact Format

- **Decision**: Use Option A compact structured text artifacts with `|`-separated key/value fields.
- **Rationale**: Best diagnostic clarity for deterministic debugging and parity review workflows, with
  lower initial implementation overhead than binary/hybrid formats.
- **Alternatives considered**:
  - Binary records + decoder (smaller files, higher tooling burden)
  - Hybrid binary+text (balanced output, highest complexity)

## Decision 2: Schema Mismatch Policy

- **Decision**: Comparator fails immediately when `schema_version` differs.
- **Rationale**: Maintains deterministic trust and prevents false comparisons across incompatible
  semantics.
- **Alternatives considered**:
  - Allow explicit override/converter path
  - Best-effort intersected comparison
  - Warning-only continuation

## Decision 3: Truncated Artifact Policy

- **Decision**: On truncated tail (missing final `FRAME_END`), compare only complete frame blocks up to
  the last valid `FRAME_END` and emit truncation warning.
- **Rationale**: Preserves utility for partial-corruption diagnostics while preventing undefined
  partial-frame interpretation.
- **Alternatives considered**:
  - Immediate hard failure on any truncation
  - Attempt partial-frame reconstruction

## Decision 4: Missing Required Row Kind Policy

- **Decision**: Missing required row kind in any frame causes immediate comparison failure.
- **Rationale**: Required row completeness is a structural contract for deterministic parity.
- **Alternatives considered**:
  - Skip invalid frame and continue
  - Infer missing rows from surrounding frames

## Decision 5: Scope and Validation Profile

- **Decision**: Scope remains single-player baseline first; visual playback validation is included in
  feature acceptance.
- **Rationale**: Matches roadmap progression while retaining a human inspection path for replay runs.
- **Alternatives considered**:
  - Keep visual playback out of this feature
  - Expand to multiplayer parity in this phase

## Decision 6: Comparator Output Contract

- **Decision**: First-divergence output includes frame index, row kind, field name, optional entity id,
  and both compared values.
- **Rationale**: Produces minimal but sufficient diagnostics for immediate regression triage.
- **Alternatives considered**:
  - Summary-only mismatch counts
  - Full per-field dump of all mismatches

## Alignment Note

Research, contract, and plan remain aligned on these invariants:
- schema mismatch => hard fail
- missing required row kind => hard fail
- truncation tail => compare complete prior frames + warning
