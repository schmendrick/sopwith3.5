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
$csProjDir = Join-Path $root "tools\csharp"
$csDll = Join-Path $csProjDir "bin\Release\net10.0\RngParityHarness.dll"

function Convert-StepLineToMap {
  param([string]$Line)
  $map = [ordered]@{}
  foreach ($segment in ($Line -split " ")) {
    if ($segment -match "=") {
      $parts = $segment -split "=", 2
      if ($parts.Length -eq 2) {
        $map[$parts[0]] = $parts[1]
      }
    }
  }
  return $map
}

function Find-FirstMismatch {
  param(
    [string[]]$CppLines,
    [string[]]$CsLines
  )

  $lineCount = [Math]::Min($CppLines.Length, $CsLines.Length)
  for ($idx = 0; $idx -lt $lineCount; $idx++) {
    $cppLine = $CppLines[$idx]
    $csLine = $CsLines[$idx]
    if ($cppLine -ceq $csLine) {
      continue
    }

    if ($cppLine.StartsWith("step=") -and $csLine.StartsWith("step=")) {
      $cppMap = Convert-StepLineToMap -Line $cppLine
      $csMap = Convert-StepLineToMap -Line $csLine
      $step = if ($cppMap.ContainsKey("step")) { $cppMap["step"] } else { "unknown" }
      foreach ($k in $cppMap.Keys) {
        if (-not $csMap.ContainsKey($k) -or $cppMap[$k] -ne $csMap[$k]) {
          return @{ Step = $step; Field = $k; Line = ($idx + 1) }
        }
      }
      foreach ($k in $csMap.Keys) {
        if (-not $cppMap.ContainsKey($k)) {
          return @{ Step = $step; Field = $k; Line = ($idx + 1) }
        }
      }
      return @{ Step = $step; Field = "unknown"; Line = ($idx + 1) }
    }

    return @{ Step = "n/a"; Field = "header"; Line = ($idx + 1) }
  }

  if ($CppLines.Length -ne $CsLines.Length) {
    return @{ Step = "n/a"; Field = "line_count"; Line = ($lineCount + 1) }
  }

  return $null
}

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
# Example overrides:
#   powershell -ExecutionPolicy Bypass -File tools/run-rng-parity.ps1 -Tokens full,computer -Steps 16,128,1024
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

    $cppLines = @($cppOut)
    $csLines = @($csOut)
    $cppText = ($cppLines -join "`n").TrimEnd()
    $csText = ($csLines -join "`n").TrimEnd()
    if ($cppText -ne $csText) {
      Write-Host "[FAIL] token=$token steps=$step (output mismatch)"
      $mismatch = Find-FirstMismatch -CppLines $cppLines -CsLines $csLines
      if ($null -ne $mismatch) {
        Write-Host "[FAIL] first_mismatch token=$token steps=$step step=$($mismatch.Step) field=$($mismatch.Field) line=$($mismatch.Line)"
      }
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
