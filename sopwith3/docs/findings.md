# Findings

## 2026-04-13

- **Corrupt SourceForge image assets:** The original SourceForge snapshot contained corrupt image files. The broken files were replaced with assets taken from a known working Sopwith 3 installation.
  - **Why this matters:** Therefore contained sprites never are used. 

- **Renderer/asset path behavior differs by build target:** The codebase has compile-time paths for multiple render backends (notably SDL and Allegro4-era paths). Allegro-oriented builds load repository PCX sprite assets when available, and can fall back to built-in/generated sprite data if files are missing/unreadable. SDL builds currently use built-in/generated sprite data by default.
  - **Why this matters:** Visual output differences can come from backend-specific asset loading behavior, not gameplay logic.
  - **Potential improvement:** Enable optional PCX loading on SDL path could be possible. Otherwise we should remove them from the repo because they are unused
