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
