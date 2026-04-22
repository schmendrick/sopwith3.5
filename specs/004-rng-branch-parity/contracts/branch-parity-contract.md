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
2. Per-step rows with fixed field order:
   - `step`
   - `randv`
   - `x`
   - `y`
   - `i`
   - `v2_type`
   - `troubled_sound_bit`
   - `explosion_type`

Canonical row example:

`step=0 randv=1234 x=100 y=200 i=1 v2_type=3 troubled_sound_bit=0 explosion_type=3`

## Parity success condition

For identical input arguments:

- C++ output and C# output are byte-identical.
- Any byte-level difference is a parity failure.

## Failure reporting requirements

On mismatch, comparison output must identify:

- case (`token`, `steps`)
- first mismatch location (`step`, `field`)
- failing side(s)

Reference mismatch tuple shape:

`first_mismatch token=<token> steps=<steps> step=<step|n/a> field=<field>`

## Scope boundaries

- This contract validates branch decisions derived from deterministic RNG-core behavior.
- It does not validate full gameplay object/frame parity.
- It extends existing RNG-core parity, not replaces it.
