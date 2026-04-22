# Replay Usage

## Record Replay

From `sopwith3` directory:

```text
sopwith3.exe -h<replay_token>
```

`-h` **records** binary replay input history (see `sopwith.cpp` option parsing). The replay token is normalized to **`basename.tape`** for binary I/O: strip trailing **`.


A text verification sidecar is written beside the tape as **`basename.<n>.sidecar`** (first run **`basename.1.sidecar`**; further runs allocate **`max(existing n)+1`** among `basename.*.sidecar` in that directory). Each file holds a full `SESSION` row plus one `FRAME_BEGIN`…`FRAME_END` block per simulated frame while the match is in progress.

Notes:
- Replay options require attached filenames (no space): `-hmyreplay`, `-vmyreplay`.
- In PowerShell, prefer `--%` to avoid argument parsing surprises, for example:
  `.\sopwith3.exe --% -hmyreplay` or `.\sopwith3.exe --% -vshort -s -i`

## Playback Replay

```text
sopwith3.exe -v<replay_token>
```

`-v` **plays back** binary tape after the same **`basename.tape`** normalization as recording. Playback also writes **`basename.<n>.sidecar`** using the same layout as recording (session identity reflects the tape basename and current options).

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
playback_option=mytest
options_parse_done
playback_open_attempt=mytest
playback_open=ok
playback_header_seed=-6357
playback_close=ok
```

*(After normalization, the engine opens the corresponding `mytest.tape` path; the log may still show the raw token.)*

## Scope Boundaries

- In scope for this baseline: single-player deterministic replay verification.
- Deferred scope: multiplayer/network parity and non-baseline replay formats.
- Schema version mismatch is treated as a hard comparison failure.

## Baseline verification (byte compare)

Build the standalone comparator from `sopwith3/src`:

```text
mingw32-make -f Makefile.msys2 replay-compare
```

**Two explicit artifacts** (exit code 0 only when files are byte-identical line-for-line):

```text
powershell -File scripts/replay/verify-baseline.ps1 -LeftArtifact <a.sidecar> -RightArtifact <b.sidecar>
```

**Basename shortcut** (single basename, no extension): discover **`basename.*.sidecar`** in the current directory, numeric sort by `n`. **Exactly two** matches ⇒ run one two-file compare. **More than two** ⇒ prints which files matched, exits non-zero — use **`replay-compare left right`** for the pair you care about (see `specs/001-baseline-replay-verification/spec.md`).

```text
.\replay-compare.exe short
```

`replay-compare.exe` is produced next to `sopwith3.exe` in the `sopwith3` directory.

### Parity notes (golden baselines and ports)

Treat two sidecar files as comparable only when the underlying run is intended to be the same:
same binary replay tape 
(same file path not required; same tape bytes and header seed matter), same
CLI-affecting options that appear in `SESSION`, and the same stretch of gameplay (artifact length and
frame count change if one run exits earlier). The tape is the source of input history; the sidecar (the `basename.n.sidecar` file beside your `basename.tape`) is a
derived trace for diffing and for future parity checks (for example a C# port replaying the same tape at
logical-frame cadence). Checked-in golden artifacts are optional until serialization and repeatability are
stable enough that two reference runs produce identical bytes under those controlled conditions.

Each `FRAME` row includes only `frame_index` (no timer remainder fields), so comparisons are not sensitive
to wall-clock pacing differences between runs.

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
