# Research: RNG Branch-Decision Parity Harness

## Decision 1: Branch parity scope for v1

- **Decision**: Include exactly two branch outputs: troubled-sound-bit and explosion-type decision derivations.
- **Rationale**: These are deterministic RNG-driven branches directly tied to current replay hardening goals.
- **Alternatives considered**:
  - Include all RNG-dependent gameplay branches immediately (rejected: too broad for branch-parity phase)
  - Keep core-only parity without branch outputs (rejected: does not advance decision-level confidence)

## Decision 2: Output schema extension strategy

- **Decision**: Extend existing line-based machine-diffable parity output format with additional branch fields per step.
- **Rationale**: Preserves current byte-comparison workflow and keeps mismatch diagnostics simple.
- **Alternatives considered**:
  - Separate JSON output format (rejected: unnecessary format migration cost)
  - Multiple output files per decision type (rejected: fragments parity review surface)

## Decision 3: Evidence continuity strategy

- **Decision**: Record branch-parity evidence as an explicit extension to existing RNG-core parity evidence.
- **Rationale**: Maintains historical traceability and shows incremental deterministic coverage growth.
- **Alternatives considered**:
  - Replace old evidence with branch-parity only (rejected: loses core parity continuity)
  - Keep branch results only in command output logs (rejected: weak long-term traceability)

## Decision 4: Case matrix growth

- **Decision**: Reuse core token set and step sizes, then add one higher-step run for branch stability confidence.
- **Rationale**: Builds on known-good core matrix while increasing branch confidence with minimal runtime increase.
- **Alternatives considered**:
  - Massive case explosion in one phase (rejected: lower signal-to-noise for first branch parity iteration)
  - No additional step range beyond current matrix (rejected: weak confidence for branch stability)

## Decision 5: Toolchain constraint handling

- **Decision**: Keep branch-parity workflow explicitly documented against MSYS2 MinGW64 + SDL 1.2 and .NET 10.
- **Rationale**: Determinism checks are only useful when maintainers can reliably reproduce setup and results.
- **Alternatives considered**:
  - Generic toolchain statements without versions (rejected: reproducibility ambiguity)
  - CI-only instructions without local path (rejected: slower debugging feedback loop)
