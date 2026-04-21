# Replay Usage

## Record Replay

From `sopwith3` directory:

```text
sopwith3.exe -h<replay_file>
```

This writes binary replay input history. Baseline artifact scaffolding additionally writes a text
state file at `<replay_file>.state.txt` during recording initialization.

Notes:
- Replay options require attached filenames (no space): `-hmy.rec`, `-vmy.rec`.
- In PowerShell, prefer `--%` to avoid argument parsing surprises, for example:
  `.\sopwith3.exe --% -hmy.rec`

## Playback Replay

```text
sopwith3.exe -v<replay_file>
```

Use playback for visual inspection and deterministic baseline checks.

## Playback Log Output

Playback and replay-option instrumentation is written to `replay-visual.log` in the `sopwith3`
working directory.

Current log lines:
- `options_parse_start` / `options_parse_done`
- `record_option=<file>` when `-h<file>` is parsed
- `playback_option=<file>` when `-v<file>` is parsed
- `playback_open_attempt=<file>`
- `playback_open=ok|fail`
- `playback_header_seed=<seed>`
- `playback_stream_end_or_error`
- `playback_close=ok|fail`

Example:

```text
options_parse_start
playback_option=mytest.rec
options_parse_done
playback_open_attempt=mytest.rec
playback_open=ok
playback_header_seed=-6357
playback_close=ok
```

## Scope Boundaries

- In scope for this baseline: single-player deterministic replay verification.
- Deferred scope: multiplayer/network parity and non-baseline replay formats.
- Schema version mismatch is treated as a hard comparison failure.

## Baseline Verification Scaffold

The baseline helper script verifies expected artifact inputs exist:

```text
powershell -File scripts/replay/verify-baseline.ps1 -LeftArtifact <a.state.txt> -RightArtifact <b.state.txt>
```

## Visual Playback Validation

Use the smoke script to prepare visual verification:

```text
powershell -File scripts/replay/test-visual-playback.ps1 -ReplayFile <replay_file>
```

For each validation run, confirm:
- replay starts in visual mode
- replay progresses without interruption
- replay completes end-to-end

## Replay Tests

Run replay scaffold tests from `sopwith3`:

```text
test.bat
```

Or from `sopwith3/src`:

```text
mingw32-make -f Makefile.msys2 replay-tests
```
