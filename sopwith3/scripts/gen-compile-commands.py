#!/usr/bin/env python3
"""
Emit compile_commands.json for the MSYS2 MinGW Makefile.msys2 build.

Runs:  mingw32-make -B -f Makefile.msys2 -n all replay-compare
from sopwith3/src and records every g++ compile line (main binary + replay-compare tool). Regenerate after
changing the makefile, flags, or source list.

Usage (from repo: any shell with mingw32-make + g++ + pkg-config on PATH):
  python sopwith3/scripts/gen-compile-commands.py

Output: <workspace-root>/compile_commands.json
"""

from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path


def _parse_gpp_line(line: str) -> tuple[str, str] | None:
    line = line.strip()
    if not line.startswith("g++"):
        return None
    marker = " -c "
    i = line.find(marker)
    if i == -1:
        return None
    tail = line[i + len(marker) :].strip()
    j = tail.rfind(" -o ")
    if j == -1:
        return None
    src = tail[:j].strip()
    if not src.endswith(".cpp"):
        return None
    return src, line


def main() -> int:
    script_dir = Path(__file__).resolve().parent
    inner = script_dir.parent
    src_dir = inner / "src"
    workspace_root = inner.parent
    out_path = workspace_root / "compile_commands.json"

    if not src_dir.is_dir():
        print(f"Expected source dir at {src_dir}", file=sys.stderr)
        return 1

    make_exe = os.environ.get("MAKE", "mingw32-make")
    env = os.environ.copy()
    # Ensure MSYS2 mingw64 tools win when running from PowerShell/cmd.
    mingw_bin = env.get("MINGW_BIN")
    if mingw_bin:
        extra = str(Path(mingw_bin))
        env["PATH"] = extra + os.pathsep + env.get("PATH", "")

    proc = subprocess.run(
        [make_exe, "-B", "-f", "Makefile.msys2", "-n", "all", "replay-compare"],
        cwd=src_dir,
        capture_output=True,
        text=True,
        env=env,
    )
    if proc.returncode != 0:
        print(proc.stderr or proc.stdout or "make failed", file=sys.stderr)
        return proc.returncode

    entries: list[dict[str, str]] = []
    seen: set[str] = set()
    for line in proc.stdout.splitlines():
        parsed = _parse_gpp_line(line)
        if not parsed:
            continue
        src_rel, command = parsed
        abs_file = (src_dir / src_rel).resolve()
        key = str(abs_file)
        if key in seen:
            continue
        seen.add(key)
        entries.append(
            {
                "directory": str(src_dir.resolve()),
                "command": command,
                "file": str(abs_file),
            }
        )

    entries.sort(key=lambda e: e["file"])
    out_path.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {len(entries)} entries to {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
