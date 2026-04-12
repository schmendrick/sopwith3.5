# Phase 1 Report: Build, Run, and Manual Smoke

## Purpose

Capture reproducible evidence that Sopwith 3.5 can be built, launched, interacted with, and exited cleanly on the current baseline environment.

## Scope

- Build reliability on current Windows/MSYS2 workflow.
- Runtime launch sanity.
- Basic control sanity.
- Verified clean quit path (no force-kill).

## Environment

- OS: Windows 11
- Date: 2026-04-13
- Repo: `sopwith3`

## Observed Workflows

- Build:
  - `build.bat`
- Launch:
  - `F5` in Cursor (launch config)
  - `.\sopwith3.exe -s -k`

## Smoke Checklist

- [x] Build succeeds.
- [x] Game launches successfully.
- [x] Basic controls respond in single-player (`-s -k` baseline).
- [x] Clean quit path works without force-killing process.

## Notes

- Manual smoke cycle required several runs/bugfixes before stabilizing.
- Network/multiplayer paths are preserved but not part of this tested baseline.

## Exit-Criteria Mapping (Roadmap Phase 1)

- Fresh machine can build and run from documented steps: **Met (current maintainer environment)**.
- Manual smoke includes launch/input/quit without force-kill: **Met**.

## Follow-up

- Keep this report updated if build/launch workflow changes.
- If environment changes substantially, re-run smoke and append a new dated entry.
