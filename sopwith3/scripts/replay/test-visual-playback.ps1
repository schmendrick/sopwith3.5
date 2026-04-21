param(
  [string]$ReplayFile
)

if (-not $ReplayFile) {
  Write-Host "Usage: test-visual-playback.ps1 -ReplayFile <file>"
  exit 1
}

if (-not (Test-Path $ReplayFile)) {
  Write-Host "Replay not found: $ReplayFile"
  exit 1
}

Write-Host "Visual playback smoke checklist:"
Write-Host "- Launch replay with sopwith3.exe -v$ReplayFile"
Write-Host "- Confirm replay starts, progresses, and exits cleanly"
exit 0
