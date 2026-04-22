# Evidence: Phase 2B Post-change Repeatability

Date: 2026-04-22  
Branch: `003-phase2b-determinism-prereqs`

## Context

- Pre-change baseline repeatability for `full.tape`, `bomb.tape`, `bird.tape`, and `computer.tape`
  was previously confirmed by maintainer.
- Post-change runs were executed with fixed-step validation enabled via
  `SOPWITH_FIXED_STEP=1` (through `scripts/replay/run-fixed-step-session.ps1`).
- Computer fixture post-change rerun was validated with computer-enemy mode args (`-vcomputer -i -c`)
  to match baseline intent.

## Post-change comparison inputs

- `full`: `full.3.sidecar` vs `full.4.sidecar`
- `bomb`: `bomb.3.sidecar` vs `bomb.4.sidecar`
- `bird`: `bird.3.sidecar` vs `bird.4.sidecar`
- `computer`: `computer.7.sidecar` vs `computer.8.sidecar`

## Comparator outcomes

- `full`: `Replay compare success` (exit code `0`)
- `bomb`: `Replay compare success` (exit code `0`)
- `bird`: `Replay compare success` (exit code `0`)
- `computer`: `Replay compare success` (exit code `0`)

## Delta classification

- `full`: `pass_to_pass`
- `bomb`: `pass_to_pass`
- `bird`: `pass_to_pass`
- `computer`: `pass_to_pass`

## Regression/improvement summary

- Regressions (`pass_to_fail`): `0`
- Improvements (`fail_to_pass`): `0`
- Stable passes (`pass_to_pass`): `4`
- Stable fails (`fail_to_fail`): `0`

## RNG core parity harness results (C++ vs C#)

### Harness artifacts

- C++ mini-exe: `sopwith3/rng-parity-cpp.exe`
- C# mini-exe project: `tools/csharp/RngParityHarness.csproj`
- parity runner: `tools/run-rng-parity.ps1`
- harness usage/repro doc: `tools/README.md`
- frozen contract: `specs/003-phase2b-determinism-prereqs/rng-core-contract.md`

### Executed command

From repo root:

```powershell
powershell -ExecutionPolicy Bypass -File tools/run-rng-parity.ps1
```

### Cases

- Tokens: `full`, `bomb`, `bird`, `computer`, `short.rec`, `short.tape`
- Step counts: `16`, `128`
- Total comparisons: `12`

### Outcome

- All `12/12` parity cases passed.
- C++ and C# output was byte-identical for every token/step pair.

### Environment notes / reproducibility

- C# harness target framework: `net10.0`.
- SDK requirement: .NET 10 (`10.0.x`) in `dotnet --list-sdks`.
- Admin install reference used for setup:

```powershell
choco install dotnet-sdk --version=10.0.203 -y
```

### Rationale

- The parity harness provides an objective pre-port gate for deterministic behavior.
- Isolating RNG-core parity from full game logic reduces debugging scope and de-risks C# implementation.

## Phase 2C gameplay parity surface (rollup pointer)

Phase 2C adds a separate gameplay parity harness and matrix runner (does not replace RNG parity). Evidence and commands for that feature live under `specs/006-gameplay-parity-surface/evidence.md` and `tools/run-gameplay-parity.ps1`.
