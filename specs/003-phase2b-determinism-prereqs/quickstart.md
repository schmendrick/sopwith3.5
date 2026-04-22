# Quickstart: Phase 2B Determinism Prerequisites

This quickstart describes how to execute the fixture matrix and record before/after repeatability
results for determinism hardening.

## Baseline capture status

Pre-change baseline capture for all four fixtures is already confirmed by the maintainer.  
Use this quickstart primarily for post-change validation unless baseline needs to be re-established.

## 1) Confirm fixture tapes are present

In the working replay directory, confirm these tapes exist:

- `full.tape`
- `bomb.tape`
- `bird.tape`
- `computer.tape`

## 2) Build executable and replay comparator

Toolchain requirement for this step:

- Windows build path uses **MSYS2 MinGW64 + SDL 1.2**
- See `Toolchain-Readme.md` for package details and environment setup

From repo root:

```powershell
.\sopwith3\sdlbuild.bat
```

From `sopwith3/src` (MINGW64 shell):

```text
mingw32-make -f Makefile.msys2 replay-compare
```

## 3) Capture pre-change baseline results

For each fixture (`full`, `bomb`, `bird`, `computer`):

1. Run replay workflow twice with equivalent settings to emit two sidecars.
2. Compare sidecars with `replay-compare.exe` (or `verify-baseline.ps1`).
3. Record pass/fail and divergence summary.

Example compare from `sopwith3` directory:

```powershell
.\replay-compare.exe full.1.sidecar full.2.sidecar
```

## 4) Implement determinism hardening changes

Apply the Phase 2B changes in planned order:

1. RNG hardening touchpoints.
2. Timer-independent fixed-step validation path.

## 5) Capture post-change results

Repeat Step 3 for all four fixtures and record outcomes using the same run protocol.

## 6) Classify fixture deltas

For each fixture, classify the change:

- `pass_to_pass`
- `fail_to_pass`
- `pass_to_fail`
- `fail_to_fail`

Any `pass_to_fail` result is a regression and blocks feature closeout.

## 7) Publish evidence set

Store fixture-level before/after summary and comparison outputs under this feature directory as part of
Phase 2B completion evidence.
