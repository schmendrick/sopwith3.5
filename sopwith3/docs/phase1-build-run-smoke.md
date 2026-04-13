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
- [ ] Build succeeds.
- [ ] Game launches successfully.
- [ ] Basic controls respond in single-player (`-s -k` baseline).
- [ ] Clean quit path works without force-killing process.


## Notes

- Manual smoke cycle (SDL) required several runs/bugfixes before stabilizing.
- Network/multiplayer paths are preserved but not part of this tested baseline.
- Allegro backend source currently targets legacy Allegro 4 APIs (`<allegro.h>`), while MSYS2 `mingw-w64-x86_64-allegro` provides Allegro 5 (`allegro5/...`), so Allegro build remains blocked in this environment.

## Exit-Criteria Mapping (Roadmap Phase 1)

- Fresh machine can build and run from documented steps on SDL: **Met for SDL (current maintainer environment)**.
- Manual smoke includes launch/input/quit without force-kill: **Met**.
- Fresh machine can build and run from documented steps on ALLEGRO: **Not met (current MSYS2 package provides Allegro 5, source backend expects Allegro 4 API/header)**.

## Follow-up

- Keep this report updated if build/launch workflow changes.
- If environment changes substantially, re-run smoke and append a new dated entry.
