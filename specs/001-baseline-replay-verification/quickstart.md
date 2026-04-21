# Quickstart: Baseline Replay Verification

## 1) Build the game executable

From repository root, use the maintained MSYS2 SDL build workflow:

```powershell
.\sopwith3\sdlbuild.bat
```

## 2) Record a replay tape (baseline input)

From `sopwith3` directory:

```powershell
.\sopwith3.exe -h<replay_file>
```

Create two runs with identical intended seed/input conditions for repeatability checks.

## 3) Generate replay verification artifacts

Run replay playback with artifact dump output enabled (flag names may match final implementation):

```powershell
.\sopwith3.exe -v<replay_file> --dump-state <artifact_file>
```

Generate at least:
- `artifact_run1.txt`
- `artifact_run2.txt`

## 4) Validate deterministic repeatability

- Compare `artifact_run1.txt` and `artifact_run2.txt`
- Expected: no divergence for identical replay and seed

## 5) Validate first-divergence behavior

Create or inject a known mismatch in one artifact and run comparator:
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
