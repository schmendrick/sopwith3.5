# Findings

## 2026-04-13

- **Corrupt SourceForge image assets:** The original SourceForge snapshot contained corrupt image files. The broken files were replaced with assets taken from a known working Sopwith 3 installation.
  - **Why this matters:** Therefore contained sprites never are used. 

- **Renderer/asset path behavior differs by build target:** The codebase has compile-time paths for multiple render backends (notably SDL and Allegro4-era paths). Allegro-oriented builds load repository PCX sprite assets when available, and can fall back to built-in/generated sprite data if files are missing/unreadable. SDL builds currently use built-in/generated sprite data by default.
  - **Why this matters:** Visual output differences can come from backend-specific asset loading behavior, not gameplay logic.
  - **Potential improvement:** Enable optional PCX loading on SDL path could be possible. Otherwise we should remove them from the repo because they are unused

## 2026-04-21

- **`-D` (“Author’s Edition”) is `version=7`, not one switch:** The CLI flag sets `version` to `7` via `getoption(..., version, 7)` in `sopwith.cpp`. Gameplay differences are everywhere the code branches on `version==2` vs `version!=2` (anything not `2` follows the alternate path, including `7`).
  - **Why this matters:** Treating `-D` as “one feature” is misleading; it is an alternate physics/combat/tuning profile spread across `plane.cpp`, `target.cpp`, `object.cpp`, `frag.cpp`, `bomb.cpp`, etc.
  - **Notable effects (non-exhaustive):** Default main-loop `speed` differs (`20` when `version==2`, else `15` in `run()`). Plane collisions can wound vs explode differently; AI/autopilot thresholds and gun lead differ; target bullet-hit and explosion paths differ; explosion velocity inherits more parent motion when not v2; distance/range helpers weight vertical differently; fragment collision rules differ from classic mode.

## 2026-04-22
- **Replay sidecar deterministic compare (playback vs playback):** Evidence that two identical playbacks yield byte-identical `.state.txt` for the same tape and CLI (schema v2 sidecars with `object_kind` on `OBJECT` rows, stable ordering, initialized `Object` fields).
  - **Tape:** `short.rec` (example; any fixed tape works).
  - **Commands run from `sopwith3/` (directory containing `sopwith3.exe` and `replay-compare.exe`):**
```powershell
.\sopwith3.exe --% -vshort.rec -s -i
copy .\short.rec.state.txt .\short.rec.state-a.txt
.\sopwith3.exe --% -vshort.rec -s -i
.\replay-compare.exe .\short.rec.state-a.txt .\short.rec.state.txt
