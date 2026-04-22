# Contract: RNG Branch-Decision Parity

## Purpose

Define deterministic branch-level parity requirements for C++ and C# harness outputs.

## Inputs

- `--token <replay-token>`
- `--steps <positive-int>`

Both implementations must accept equivalent input semantics.

## Required output rows

Each run must emit:

1. Header rows:
   - token
   - normalized token
   - seed
2. Per-step rows with fixed field order including:
   - `step`
   - `randv`
   - `x`
   - `y`
   - `i`
   - `v2_type`
   - `troubled_sound_bit`
   - `explosion_type`

## Parity success condition

For identical input arguments:

- C++ output and C# output are byte-identical.
- Any byte-level difference is a parity failure.

## Failure reporting requirements

On mismatch, comparison output must identify:

- case (`token`, `steps`)
- first mismatch location (`step`, `field` when derivable)
- failing side(s)

## Scope boundaries

- This contract validates branch decisions derived from deterministic RNG-core behavior.
- It does not validate full gameplay object/frame parity.
- It extends existing RNG-core parity, not replaces it.
