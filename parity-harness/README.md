# RNG Core Parity Harness

## Purpose

This harness validates deterministic RNG-core parity between:

- C++ executable: `sopwith3/rng-parity-cpp.exe`
- C# executable: `parity-harness/csharp` (`net10.0`)

It verifies that both languages produce byte-identical output for:

1. replay-token normalization
2. deterministic seed derivation
3. stepwise `randv` sequence and derived v2 type output

The contract source of truth is:

- `specs/003-phase2b-determinism-prereqs/rng-core-contract.md`

## Prerequisites

- Native toolchain: **MSYS2 MinGW64 + SDL 1.2** (same as the main game build path)
- `mingw32-make` available from the MSYS2 MinGW64 environment
- .NET 10 SDK installed (`10.0.x`)

For the full native toolchain setup, see `Toolchain-Readme.md`.

Admin install command (Chocolatey):

```powershell
choco install dotnet-sdk --version=10.0.203 -y
```

Verify:

```powershell
dotnet --list-sdks
```

## Reproduce parity checks

From repository root:

```powershell
powershell -ExecutionPolicy Bypass -File parity-harness/run-rng-parity.ps1
```

Default matrix:

- Tokens: `full`, `bomb`, `bird`, `computer`, `short.rec`, `short.tape`
- Steps: `16`, `128`

The script builds both harnesses and fails fast on first mismatch.

## Rationale

- C# port parity needs deterministic low-level agreement before gameplay/system parity.
- A tiny side-by-side harness isolates numeric width/overflow semantics from game-loop complexity.
- Byte-identical output gives an objective parity gate in CI/local workflows.

## Troubleshooting

- `NETSDK1045` (target framework unsupported): install .NET 10 SDK and restart terminal/IDE.
- File lock (`CS2012`, `VBCSCompiler`): rerun command or close processes holding `obj/bin` outputs.
- If PATH does not refresh after install: restart terminal/IDE session.
