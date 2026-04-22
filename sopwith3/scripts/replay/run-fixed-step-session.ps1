param(
  [Parameter(Mandatory = $true)]
  [string]$ReplayToken,
  [ValidateSet("playback", "record")]
  [string]$Mode = "playback",
  [int]$RepeatCount = 2,
  [string[]]$ExtraArgs = @("-s", "-i")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent (Split-Path -Parent $scriptDir)
$exePath = Join-Path $repoRoot "sopwith3.exe"

if (-not (Test-Path $exePath)) {
  Write-Host "sopwith3.exe not found at: $exePath"
  Write-Host "Build first (repo root): .\sopwith3\sdlbuild.bat"
  exit 1
}

if ($RepeatCount -lt 1) {
  Write-Host "RepeatCount must be >= 1"
  exit 1
}

$flag = if ($Mode -eq "record") { "-h$ReplayToken" } else { "-v$ReplayToken" }

$oldFixed = [Environment]::GetEnvironmentVariable("SOPWITH_FIXED_STEP", "Process")
[Environment]::SetEnvironmentVariable("SOPWITH_FIXED_STEP", "1", "Process")
try {
  for ($i = 1; $i -le $RepeatCount; $i++) {
    $args = @($flag) + $ExtraArgs
    Write-Host ("[{0}/{1}] Running fixed-step replay: {2}" -f $i, $RepeatCount, ($args -join " "))
    & $exePath @args
    if ($LASTEXITCODE -ne 0) {
      Write-Host ("Run failed with exit code {0}" -f $LASTEXITCODE)
      exit $LASTEXITCODE
    }
  }
}
finally {
  [Environment]::SetEnvironmentVariable("SOPWITH_FIXED_STEP", $oldFixed, "Process")
}

Write-Host "Fixed-step replay session complete."
