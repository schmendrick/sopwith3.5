# Quickstart: Phase 2C Gameplay Parity Surface

This quickstart describes the maintainer workflow for **gameplay decision parity** between:

- C++ executable: `sopwith3/gameplay-parity-cpp.exe` (target `gameplay-parity-cpp` in `sopwith3/src/Makefile.msys2`)
- C# executable: `tools/csharp/GameplayParityHarness` (`net10.0`, output under `bin/Release/net10.0/`)

The contract source of truth is:

- `specs/006-gameplay-parity-surface/contracts/gameplay-parity-contract.md`

Related existing contracts (must remain intact):

- `specs/003-phase2b-determinism-prereqs/rng-core-contract.md`
- `specs/004-rng-branch-parity/contracts/branch-parity-contract.md`

## Prerequisites

- Windows 11 maintainer machine (byte parity is intentionally scoped to this environment; see `specs/006-gameplay-parity-surface/spec.md`)
- MSYS2 MinGW64 + SDL 1.2 native toolchain (see `Toolchain-Readme.md`)
- .NET 10 SDK (`dotnet --list-sdks` includes `10.0.x`)

## 1) Build parity harnesses

From repository root:

```powershell
powershell -ExecutionPolicy Bypass -File tools/run-gameplay-parity.ps1 -SkipBuild:$false
```

Native build requires `mingw32-make` on PATH (MSYS2 MinGW64), same as `tools/run-rng-parity.ps1`.

## 2) Run the fixed matrix

```powershell
powershell -ExecutionPolicy Bypass -File tools/run-gameplay-parity.ps1
```

Optional: custom matrix path:

```powershell
powershell -ExecutionPolicy Bypass -File tools/run-gameplay-parity.ps1 -MatrixPath "F:\path\to\gameplay-parity-matrix.json"
```

Expected:

- all configured cases pass
- final line: `Gameplay parity check passed for all cases.`
- failures print `first_mismatch matrix_version=... case_id=... logical_step=... category=... field=... line=...`

## 3) Keep evidence reproducible when tapes evolve

When any referenced tape changes or the matrix grows, bump `matrix_version` in the matrix file **before** claiming new evidence.

Update:

- `specs/006-gameplay-parity-surface/evidence.md` (create during implementation if missing)
- optionally extend `specs/003-phase2b-determinism-prereqs/evidence.md` with a pointer section (if you want a single “determinism program” rollup)

## 4) Regression: ensure Phase 2B/004 harnesses still pass

Gameplay parity work must not break RNG parity gates:

```powershell
powershell -ExecutionPolicy Bypass -File tools/run-rng-parity.ps1
```
