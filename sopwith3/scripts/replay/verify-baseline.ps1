param(
  [string]$LeftArtifact,
  [string]$RightArtifact
)

if (-not $LeftArtifact -or -not $RightArtifact) {
  Write-Host "Usage: verify-baseline.ps1 -LeftArtifact <file> -RightArtifact <file>"
  exit 1
}

if (-not (Test-Path $LeftArtifact)) {
  Write-Host "Missing left artifact: $LeftArtifact"
  exit 1
}

if (-not (Test-Path $RightArtifact)) {
  Write-Host "Missing right artifact: $RightArtifact"
  exit 1
}

Write-Host "Baseline replay verification scaffold ready."
Write-Host "Left:  $LeftArtifact"
Write-Host "Right: $RightArtifact"
exit 0
