param(
  [string]$LeftArtifact,
  [string]$RightArtifact
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent (Split-Path -Parent $scriptDir)
$compareExe = Join-Path $repoRoot "replay-compare.exe"

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

if (-not (Test-Path $compareExe)) {
  Write-Host "replay-compare.exe not found at: $compareExe"
  Write-Host "Build it from sopwith3/src: mingw32-make -f Makefile.msys2 replay-compare"
  exit 1
}

Write-Host "Comparing artifacts with replay-compare.exe"
Write-Host "Left:  $LeftArtifact"
Write-Host "Right: $RightArtifact"

& $compareExe $LeftArtifact $RightArtifact
exit $LASTEXITCODE
