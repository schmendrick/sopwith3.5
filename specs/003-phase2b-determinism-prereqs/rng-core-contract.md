# RNG Core Contract (C++ to C# Parity)

## Purpose

This contract defines the deterministic RNG-core behavior to be mirrored in both the C++ and C# parity
harness executables.

## 1) Replay token normalization

Given replay token `t`:

1. Split into `prefix` + `basename` at last `/` or `\`.
2. Repeatedly strip trailing `.tape` or `.rec` from `basename` (case-insensitive), one suffix per loop,
   until neither suffix matches.
3. Rebuild normalized path as `prefix + basename + ".tape"`.

## 2) Seed derivation

Seed is derived from normalized replay path with FNV-1a style hashing:

- `h` starts at `2166136261` (`uint32`)
- For each byte `b` in normalized path:
  - `h = h XOR b`
  - `h = h * 16777619` (wrap at 32-bit unsigned)
- `s = h & 0x7fff`
- If `s == 0`, force `s = 1`
- Final seed type: signed 16-bit (`short`) value in range `[1, 32767]`

## 3) Legacy randv update step

Legacy update (used in non-v2 explosion path) is part of parity scope for arithmetic verification:

- `next = to_int16(x * y * current + 7491)`
- If `next == 0`, set `next = to_int16(74917777)`

Where:

- `current`, `next` are signed 16-bit values
- `x`, `y` are signed 32-bit inputs
- `to_int16(v)` means wrap to low 16 bits then interpret as signed 16-bit

## 4) v2 explosion-type derived value

v2 explosion-type path does not mutate `randv` and derives type with:

- `randv1 = x * y * randv * 7491` (signed 32-bit arithmetic)
- `u16 = low_16_bits(randv1 * i)` interpreted as unsigned 16-bit
- `type = ((u16 * 8) >> 16)` in unsigned arithmetic

Harness parity includes this derived output to verify cross-language integer-width behavior.

## 5) Harness sequence vectors

For deterministic parity tests over `N` steps, both languages use:

- `x(n) = 100 + 13*n`
- `y(n) = 200 + 7*n`
- `i(n) = 1 + (n % 15)`

At each step `n`:

1. Emit current `randv`
2. Emit `v2_type(n)` from Section 4 using current `randv`, `x(n)`, `y(n)`, `i(n)`
3. Update `randv` with legacy step from Section 3 using `x(n)`, `y(n)`

## 6) C# mapping rules

- Use `unchecked` for arithmetic that must wrap.
- Use explicit casts to `short`, `ushort`, `uint` at every width boundary.
- Use identical input byte encoding for token hashing (ASCII/UTF-8 bytes of normalized path).

## 7) Expected parity condition

For identical `--token` and `--steps` arguments, C++ and C# harness outputs must be byte-identical.
