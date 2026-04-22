# Evidence: Phase 2C Gameplay Parity Surface

## Matrix version

- **matrix_version**: `006.1` (see `tools/gameplay-parity-matrix.json`)

## Commands run

```powershell
# Gameplay parity
powershell -ExecutionPolicy Bypass -File tools/run-gameplay-parity.ps1

# Phase 2B/004 non-regression (RNG parity)
powershell -ExecutionPolicy Bypass -File tools/run-rng-parity.ps1
```

## Per-case results

| case_id | matrix_version | result | notes |
|---------|------------------|--------|-------|
| smoke-full-16 | 006.1 | PASS | 2026-04-22 local run |
| smoke-bomb-16 | 006.1 | PASS | 2026-04-22 local run |
| smoke-computer-128 | 006.1 | PASS | 2026-04-22 local run |

## Mismatch appendix

_(None — or paste `first_mismatch ...` tuple + repro notes here.)_

## Phase 2B/004 non-regression

| gate | command | result | notes |
|------|-----------|--------|-------|
| RNG parity | `tools/run-rng-parity.ps1` | PASS | 2026-04-22 local run; all default token/step pairs |
