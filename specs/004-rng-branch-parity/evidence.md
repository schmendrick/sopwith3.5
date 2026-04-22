# Evidence: RNG Branch-Decision Parity

Date: 2026-04-22  
Branch: `004-rng-branch-parity`

## Acceptance Checklist

- [x] Command reproducibility documented and verified
- [x] Case matrix listed (tokens + steps)
- [x] Pass/fail totals recorded
- [x] Mismatch location format recorded (`token`, `steps`, `step`, `field`)
- [x] Continuity note links this evidence to RNG-core parity evidence

## Execution Command

From repository root:

```powershell
powershell -ExecutionPolicy Bypass -File tools/run-rng-parity.ps1 -Steps 16,128,1024
```

## Case Matrix

- Tokens: `full`, `bomb`, `bird`, `computer`, `short.rec`, `short.tape`
- Steps: `16`, `128`, `1024`
- Total expected comparisons: `18`

## Results

- Outcome: `18/18` cases passed.
- Failures: `0`.
- Global summary line observed in all runs: `RNG parity check passed for all cases.`

### Executed pass set (maintainer-run)

Step `16`:

- `full`, `bomb`, `bird`, `computer`, `short.rec`, `short.tape` all passed.

Step `128`:

- `full`, `bomb`, `bird`, `computer`, `short.rec`, `short.tape` all passed.

Step `1024`:

- `full`, `bomb`, `bird`, `computer`, `short.rec`, `short.tape` all passed.

## Mismatch Reporting Format

If any mismatch occurs, runner output includes:

- `token=<token>`
- `steps=<steps>`
- `step=<step|n/a>`
- `field=<field>`

Observed status for this run: no mismatches reported.

## Continuity Note

This branch-level parity evidence extends (does not replace) RNG-core evidence in:

- `specs/003-phase2b-determinism-prereqs/evidence.md`
