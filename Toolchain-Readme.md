# Toolchain (build and run)

This document describes how to compile and run **Sopwith 3.5** from source. A fuller project readme will be added separately. (Directory and executable names may still say `sopwith3` from the upstream layout.)

**Environment:** these steps were tested on **Windows 11**.

## What you need

- **MSYS2** — provides a MinGW-w64 toolchain and packages. Install from [msys2.org](https://www.msys2.org/) (default install path assumed below is `C:\msys64`; if yours differs, adjust paths in `sopwith3/src/Makefile.msys2` where `MINGW_BIN`, `CP`, and `RMCMD` are set).
- **MINGW64** packages (install from an **MSYS2 MinGW 64-bit** terminal, not MSYS):
  - `mingw-w64-x86_64-gcc` (includes `g++`)
  - `mingw-w64-x86_64-make` (provides `mingw32-make`)
  - `mingw-w64-x86_64-pkg-config`
  - `mingw-w64-x86_64-SDL` — **SDL 1.2** (the build uses `pkg-config` flags for `sdl`)

Network multiplayer in this tree may rely on HawkNL in other configurations; the **Makefile.msys2** build is set up for **local play without HawkNL** (`-DSDL` only).

## Build (recommended: `Makefile.msys2`)

1. Open **MSYS2 MinGW 64-bit** from the Start menu.
2. Go to the source directory (the repo has an inner `sopwith3` folder; sources live in `sopwith3/src`):

   ```bash
   cd /path/to/your/clone/sopwith3/src
   ```

   In MSYS2, a Windows path like `F:\work\sopwith3` becomes `/f/work/sopwith3`.

3. Build:

   ```bash
   mingw32-make -f Makefile.msys2
   ```

4. On success, the executable is written as **`sopwith3/sopwith3.exe`** (one level above `src`). The makefile also copies **`SDL.dll`** and **`libwinpthread-1.dll`** next to the `.exe` so you can run from Explorer without MSYS2 on `PATH`.

5. Clean:

   ```bash
   mingw32-make -f Makefile.msys2 clean
   ```

## Run

The game loads assets with paths such as `data/images/...` relative to the **current working directory**. Run **`sopwith3.exe`** from the inner **`sopwith3`** folder (the one that contains both **`sopwith3.exe`** and the **`data`** directory), for example:

```bash
cd /path/to/your/clone/sopwith3
./sopwith3.exe
```

Or start it from that folder in File Explorer. If you move only the `.exe` without the `data` tree beside it, graphics and sound will not load correctly.

## Other makefiles

- **`Makefile.win`** — legacy **Dev-C++**-style makefile (SDL + HawkNL). It expects a Dev-C++/MinGW layout and is not the maintained path; use **`Makefile.msys2`** unless you are deliberately reviving that setup.

## Troubleshooting

- **`pkg-config: command not found` or missing SDL flags:** install `mingw-w64-x86_64-pkg-config` and `mingw-w64-x86_64-SDL`, and ensure you are in the **MINGW64** shell, not plain MSYS.
- **`mingw32-make` not found:** install `mingw-w64-x86_64-make`.
- **Wrong shell:** building with the MSYS (Cygwin-like) environment instead of **MINGW64** often breaks MinGW paths and `pkg-config` output.
