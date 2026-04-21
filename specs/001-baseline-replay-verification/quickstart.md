# Quickstart: Baseline Replay Verification

## 1) Build the game executable

From repository root, use the maintained MSYS2 SDL build workflow:

```powershell
.\sopwith3\sdlbuild.bat
```

## 2) Build the replay comparator (optional but required for script compare)

From `sopwith3/src` in a MINGW64 shell:

```text
mingw32-make -f Makefile.msys2 replay-compare
```

This places `replay-compare.exe` next to `sopwith3.exe` in the `sopwith3` directory.

## 3) Record or play back a replay tape

From `sopwith3` directory:

```powershell
.\sopwith3.exe --% -hmy.rec
```

or playback:

```powershell
.\sopwith3.exe --% -vmy.rec
```

Each run writes `my.rec.state.txt` beside the tape: one `SESSION` row (including `gamemode`,
`session_id`, `version`, `rules_version`, seed, latency, player index) and full per-frame blocks for
every simulation frame while the session is playing.

## 4) Validate deterministic repeatability

Record or play the same tape twice with identical CLI flags and seed behavior, then compare artifacts:

```powershell
powershell -File sopwith3/scripts/replay/verify-baseline.ps1 -LeftArtifact run1.rec.state.txt -RightArtifact run2.rec.state.txt
```

Expected: exit code 0 and `Replay compare success` when outputs are byte-identical.

## 5) Validate first-divergence behavior

Create or inject a known mismatch in one artifact and run `replay-compare.exe` directly:
- Expected: first mismatch output with `frame_index`, `row_kind`, `field_name`, `lhs_value`, `rhs_value`

## 6) Validate structural failure rules

- Schema mismatch test: change `schema_version` in one artifact => comparison fails
- Missing required row kind test: remove required row in one frame => comparison fails
- Truncated tail test: remove final `FRAME_END` => compare complete frames only + truncation warning

## 7) Validate visual playback path

Open the same replay for human inspection:

```powershell
.\sopwith3.exe -v<replay_file>
```

Expected:
- Replay starts
- Replay progresses frame-to-frame
- Replay can be inspected end-to-end without replay-flow interruption

## 8) Capture validation evidence

- Store command outputs and comparison summaries for the run.
- Record artifact filenames and replay file used.
- Record whether truncated-tail warning path was exercised.
