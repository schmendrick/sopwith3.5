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

## Decision 7: Canonical binary tape filename

- **Decision**: Normalize replay tokens (**`-h`/`-v`** attached string) so binary I/O always targets **`<stripped-basename>.tape`** in the replay file’s directory: repeatedly strip trailing **`.tape`** / **`.rec`** from the basename (case-insensitive), then append **`.tape`**.
- **Rationale**: Eliminates **`short`** vs **`short.rec`** vs **`short.tape`** ambiguity; matches normative **spec.md** §Tape path normalization.
- **Alternatives considered**:
  - Prefer caller’s extension as authoritative (rejected: breaks single canonical tape identity)
  - Separate `--tape-path` flag (rejected: scope creep vs attached-token UX)

## Decision 8: Sidecar filename allocation

- **Decision**: Emit verification text to **`basename.<positive-n>.sidecar`** beside the normalized tape directory; **`n := max(existing n for basename.*.sidecar)+1`** (scan all valid integer **n**, gaps allowed), or **1** when none exist.
- **Rationale**: Preserves history of runs without overwriting; aligns with determinism regression workflows (compare **n=1** vs **n=2** from sequential runs).
- **Alternatives considered**:
  - Timestamp-based names (less sortable / stable in scripts)
  - Fill lowest gap first (spec explicitly chose **max+1**)

## Decision 9: Single-basename replay-compare discovery

- **Decision**: Single CLI argument (**basename**, no extension) discovers **`basename.<n>.sidecar`** in cwd, numeric sort by **n**, prints all matched paths. **Exactly two** matches ⇒ one two-file compare. **&lt;2** ⇒ non-zero. **&gt;2** ⇒ print matches only, non-zero, **no** comparison sweep (maintainer uses **`replay-compare left right`** for a chosen pair).
- **Rationale**: Avoid \(\binom{N}{2}\) runs and ambiguous pair selection; listing extra captures is enough for diagnostics.
- **Alternatives considered**:
  - Exhaustive pairwise comparison for **N≥3** (rejected: user scope — not needed)
  - Compare two lowest **n** when **N&gt;2** (rejected: silent choice, surprising)

## Alignment Note

Research, contract, and plan remain aligned on these invariants:
- schema mismatch => hard fail
- missing required row kind => hard fail
- truncation tail => compare complete prior frames + warning
- canonical **`.tape`** path for binary replay I/O
- numbered **`*.sidecar`** via **max(n)+1**
- single-basename **`replay-compare`**: compare **only** when **exactly two** matches; **&gt;2** ⇒ list and error
