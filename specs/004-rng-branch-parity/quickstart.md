# Quickstart: RNG Branch-Decision Parity Harness

This quickstart verifies deterministic branch-level parity (troubled sound bit and explosion type)
between C++ and C# harness implementations.

## Prerequisites

- Windows 11
- MSYS2 MinGW64 + SDL 1.2 native toolchain (see `Toolchain-Readme.md`)
- .NET 10 SDK (`dotnet --list-sdks` includes `10.0.x`)

## 1) Build parity harnesses

From repository root:

```powershell
powershell -ExecutionPolicy Bypass -File parity-harness/run-rng-parity.ps1 -SkipBuild:$false
```

This builds:

- `sopwith3/rng-parity-cpp.exe`
- `parity-harness/csharp` release output

## 2) Run branch-parity matrix

Use default matrix:

```powershell
powershell -ExecutionPolicy Bypass -File parity-harness/run-rng-parity.ps1
```

Expected:

- all configured cases pass
- final line indicates global parity pass

## 3) Expand confidence run

Run with an additional higher step count (example):

```powershell
powershell -ExecutionPolicy Bypass -File parity-harness/run-rng-parity.ps1 -Steps 16,128,1024
```

## 4) Record evidence

Update branch parity evidence summary under this feature directory with:

- case matrix
- pass/fail totals
- any mismatch details (if present)
- rationale for scope and confidence level
