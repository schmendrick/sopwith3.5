param(
  [string]$MatrixPath = "",
  [switch]$SkipBuild
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$cppSrcDir = Join-Path $root "sopwith3\src"
$cppExe = Join-Path $root "sopwith3\gameplay-parity-cpp.exe"
$csProjDir = Join-Path $root "tools\csharp\GameplayParityHarness"
$csDll = Join-Path $csProjDir "bin\Release\net10.0\GameplayParityHarness.dll"
if ([string]::IsNullOrWhiteSpace($MatrixPath)) {
  $MatrixPath = Join-Path $root "tools\gameplay-parity-matrix.json"
}

function Convert-LineToMap {
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

    if ($cppLine.StartsWith("decision ") -and $csLine.StartsWith("decision ")) {
      $cppMap = Convert-LineToMap -Line $cppLine
      $csMap = Convert-LineToMap -Line $csLine
      $step = if ($cppMap.Contains("logical_step")) { $cppMap["logical_step"] } else { "unknown" }
      $category = if ($cppMap.Contains("category")) { $cppMap["category"] } else { "n/a" }
      foreach ($k in $cppMap.Keys) {
        if (-not $csMap.ContainsKey($k) -or $cppMap[$k] -cne $csMap[$k]) {
          return @{ Step = $step; Category = $category; Field = $k; Line = ($idx + 1) }
        }
      }
      foreach ($k in $csMap.Keys) {
        if (-not $cppMap.ContainsKey($k)) {
          return @{ Step = $step; Category = $category; Field = $k; Line = ($idx + 1) }
        }
      }
      return @{ Step = $step; Category = $category; Field = "unknown"; Line = ($idx + 1) }
    }

    if ($cppLine.StartsWith("tape_ref=") -or $csLine.StartsWith("tape_ref=")) {
      return @{ Step = "n/a"; Category = "n/a"; Field = "tape_ref"; Line = ($idx + 1) }
    }

    if ($cppLine.StartsWith("tool=") -or $csLine.StartsWith("tool=")) {
      return @{ Step = "n/a"; Category = "n/a"; Field = "tool"; Line = ($idx + 1) }
    }

    $cppMap = Convert-LineToMap -Line $cppLine
    $csMap = Convert-LineToMap -Line $csLine
    foreach ($k in $cppMap.Keys) {
      if (-not $csMap.ContainsKey($k) -or $cppMap[$k] -cne $csMap[$k]) {
        return @{ Step = "n/a"; Category = "n/a"; Field = $k; Line = ($idx + 1) }
      }
    }
    return @{ Step = "n/a"; Category = "n/a"; Field = "header"; Line = ($idx + 1) }
  }

  if ($CppLines.Length -ne $CsLines.Length) {
    return @{ Step = "n/a"; Category = "n/a"; Field = "line_count"; Line = ($lineCount + 1) }
  }

  return $null
}

if (-not $SkipBuild) {
  Push-Location $cppSrcDir
  try {
    & mingw32-make -f Makefile.msys2 gameplay-parity-cpp
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  }
  finally {
    Pop-Location
  }

  Push-Location $csProjDir
  try {
    & dotnet build "GameplayParityHarness.csproj" -c Release
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  }
  finally {
    Pop-Location
  }
}

if (-not (Test-Path $cppExe)) {
  Write-Host "Missing C++ gameplay parity executable: $cppExe"
  exit 1
}
if (-not (Test-Path $csDll)) {
  Write-Host "Missing C# gameplay parity executable: $csDll"
  Write-Host "Ensure .NET 10 SDK is installed and build succeeded."
  exit 1
}
if (-not (Test-Path $MatrixPath)) {
  Write-Host "Missing matrix file: $MatrixPath"
  exit 1
}

$matrix = Get-Content -LiteralPath $MatrixPath -Raw | ConvertFrom-Json
$matrixVersion = [string]$matrix.matrix_version
if ([string]::IsNullOrWhiteSpace($matrixVersion)) {
  Write-Host "Matrix file missing matrix_version: $MatrixPath"
  exit 1
}

$failures = 0
foreach ($case in $matrix.cases) {
  $caseId = [string]$case.case_id
  $token = [string]$case.token
  $steps = [int]$case.steps
  $argList = @(
    "--case", $caseId,
    "--matrix-version", $matrixVersion,
    "--token", $token,
    "--steps", "$steps",
    "--matrix-file", $MatrixPath
  )

  $cppOut = & $cppExe @argList
  $cppCode = $LASTEXITCODE
  $csOut = & dotnet $csDll @argList
  $csCode = $LASTEXITCODE

  if ($cppCode -ne 0 -or $csCode -ne 0) {
    Write-Host "[FAIL] case_id=$caseId (process exit mismatch C++=$cppCode C#=$csCode)"
    $failures++
    continue
  }

  $cppLines = @($cppOut)
  $csLines = @($csOut)
  $cppText = ($cppLines -join "`n").TrimEnd()
  $csText = ($csLines -join "`n").TrimEnd()
  if ($cppText -cne $csText) {
    Write-Host "[FAIL] case_id=$caseId (output mismatch)"
    $mismatch = Find-FirstMismatch -CppLines $cppLines -CsLines $csLines
    if ($null -ne $mismatch) {
      Write-Host ("[FAIL] first_mismatch matrix_version=$matrixVersion case_id=$caseId logical_step=$($mismatch.Step) category=$($mismatch.Category) field=$($mismatch.Field) line=$($mismatch.Line)")
    }
    Write-Host "--- C++ ---"
    Write-Host $cppText
    Write-Host "--- C# ---"
    Write-Host $csText
    $failures++
  }
  else {
    Write-Host "[PASS] case_id=$caseId"
  }
}

if ($failures -gt 0) {
  Write-Host "Gameplay parity check failed: $failures case(s)."
  exit 1
}

Write-Host "Gameplay parity check passed for all cases."
exit 0
