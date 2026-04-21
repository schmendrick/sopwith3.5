# Replay Usage

## Record Replay

From `sopwith3` directory:

```text
sopwith3.exe -h<replay_file>
```

This writes binary replay input history. Baseline artifact scaffolding additionally writes a text
state file at `<replay_file>.state.txt` during recording initialization.

## Playback Replay

```text
sopwith3.exe -v<replay_file>
```

Use playback for visual inspection and deterministic baseline checks.

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
