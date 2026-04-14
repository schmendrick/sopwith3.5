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
  - `sdlbuild.bat`
  - `allegrobuild.bat`
- Launch:
  - `F5` in Cursor (launch config)
  - `.\sopwith3.exe -s -k`

## Smoke Checklist

### SDL
- [x] Build succeeds.
- [x] Game launches successfully.
- [x] Basic controls respond in single-player (`-s -k` baseline).
- [x] Clean quit path works without force-killing process.

### Allegro
- [x] Build succeeds.
- [ ] Game launches successfully (FAILED).
- [ ] Basic controls respond in single-player (`-s -k` baseline) (FAILED).
- [ ] Clean quit path works without force-killing process (FAILED).


## Notes

- Manual smoke cycle (SDL) required several runs/bugfixes before stabilizing.
- Network/multiplayer paths are preserved but not part of this tested baseline.
- Allegro backend source targets legacy Allegro 4 APIs (`<allegro.h>`); MSYS2 Allegro 5 package was removed, Allegro 4.4.3.1 was built from source and installed in MINGW64, and `allegrobuild.bat` now succeeds.
- Allegro fails to run successfully. It builds but does not render correctly, AI unable to fix it, so decision is taken to remove that optional goal of making allegro4 runnable.

## Exit-Criteria Mapping (Roadmap Phase 1)

- Fresh machine can build and run from documented steps on SDL: **Met for SDL (current maintainer environment)**.
- Manual smoke includes launch/input/quit without force-kill: **Met for SDL; not met for Allegro runtime**.
- Fresh machine can build and run from documented steps on ALLEGRO: **Build met on current maintainer environment; runtime/manual smoke check FAILED**.

## Follow-up

- Keep this report updated if build/launch workflow changes.
- If environment changes substantially, re-run smoke and append a new dated entry.
