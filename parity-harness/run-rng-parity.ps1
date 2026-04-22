param(
  [string[]]$Tokens = @("full", "bomb", "bird", "computer", "short.rec", "short.tape"),
  [int[]]$Steps = @(16, 128),
  [switch]$SkipBuild
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$cppSrcDir = Join-Path $root "sopwith3\src"
$cppExe = Join-Path $root "sopwith3\rng-parity-cpp.exe"
$csProjDir = Join-Path $root "parity-harness\csharp"
$csDll = Join-Path $csProjDir "bin\Release\net10.0\RngParityHarness.dll"

if (-not $SkipBuild) {
  Push-Location $cppSrcDir
  try {
    & mingw32-make -f Makefile.msys2 rng-parity-cpp
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  }
  finally {
    Pop-Location
  }

  Push-Location $csProjDir
  try {
    & dotnet build "RngParityHarness.csproj" -c Release
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  }
  finally {
    Pop-Location
  }
}

if (-not (Test-Path $cppExe)) {
  Write-Host "Missing C++ parity executable: $cppExe"
  exit 1
}
if (-not (Test-Path $csDll)) {
  Write-Host "Missing C# parity executable: $csDll"
  Write-Host "Ensure .NET 10 SDK is installed and build succeeded."
  exit 1
}

$failures = 0
foreach ($token in $Tokens) {
  foreach ($step in $Steps) {
    $cppOut = & $cppExe --token $token --steps $step
    $cppCode = $LASTEXITCODE
    $csOut = & dotnet $csDll --token $token --steps $step
    $csCode = $LASTEXITCODE

    if ($cppCode -ne 0 -or $csCode -ne 0) {
      Write-Host "[FAIL] token=$token steps=$step (process exit mismatch C++=$cppCode C#=$csCode)"
      $failures++
      continue
    }

    $cppText = ($cppOut -join "`n").TrimEnd()
    $csText = ($csOut -join "`n").TrimEnd()
    if ($cppText -ne $csText) {
      Write-Host "[FAIL] token=$token steps=$step (output mismatch)"
      Write-Host "--- C++ ---"
      Write-Host $cppText
      Write-Host "--- C# ---"
      Write-Host $csText
      $failures++
    }
    else {
      Write-Host "[PASS] token=$token steps=$step"
    }
  }
}

if ($failures -gt 0) {
  Write-Host "RNG parity check failed: $failures case(s)."
  exit 1
}

Write-Host "RNG parity check passed for all cases."
exit 0
