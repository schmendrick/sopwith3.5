Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent (Split-Path -Parent $scriptDir)
$srcDir = Join-Path $repoRoot "src"
$buildDir = Join-Path $srcDir "build\sdl\replay-tests"
$objDir = Join-Path $buildDir "obj"
$binDir = Join-Path $buildDir "bin"

New-Item -ItemType Directory -Force -Path $objDir | Out-Null
New-Item -ItemType Directory -Force -Path $binDir | Out-Null

$testCases = @(
  @{
    Name = "test_replay_parser"
    Source = Join-Path $repoRoot "tests\replay\test_replay_parser.cpp"
    ExtraObjects = @(
      (Join-Path $srcDir "build\sdl\replay_contract.o"),
      (Join-Path $srcDir "build\sdl\replay_ordering.o"),
      (Join-Path $srcDir "build\sdl\replay_validate.o")
    )
    Args = @()
  },
  @{
    Name = "test_artifact_generation"
    Source = Join-Path $repoRoot "tests\replay\test_artifact_generation.cpp"
    ExtraObjects = @(
      (Join-Path $srcDir "build\sdl\replay_contract.o")
    )
    Args = @()
  },
  @{
    Name = "test_artifact_repeatability"
    Source = Join-Path $repoRoot "tests\replay\test_artifact_repeatability.cpp"
    ExtraObjects = @()
    Args = @(
      (Join-Path $buildDir "same-left.txt"),
      (Join-Path $buildDir "same-right.txt")
    )
  },
  @{
    Name = "test_schema_mismatch"
    Source = Join-Path $repoRoot "tests\replay\test_schema_mismatch.cpp"
    ExtraObjects = @(
      (Join-Path $srcDir "build\sdl\replay_validate.o"),
      (Join-Path $srcDir "build\sdl\replay_contract.o")
    )
    Args = @()
  },
  @{
    Name = "test_missing_required_row"
    Source = Join-Path $repoRoot "tests\replay\test_missing_required_row.cpp"
    ExtraObjects = @(
      (Join-Path $srcDir "build\sdl\replay_validate.o"),
      (Join-Path $srcDir "build\sdl\replay_contract.o")
    )
    Args = @()
  },
  @{
    Name = "test_first_divergence"
    Source = Join-Path $repoRoot "tests\replay\test_first_divergence.cpp"
    ExtraObjects = @(
      (Join-Path $srcDir "build\sdl\replay_compare.o"),
      (Join-Path $srcDir "build\sdl\replay_compare_result.o"),
      (Join-Path $srcDir "build\sdl\replay_compare_diff.o"),
      (Join-Path $srcDir "build\sdl\replay_validate.o"),
      (Join-Path $srcDir "build\sdl\replay_contract.o")
    )
    Args = @()
  },
  @{
    Name = "test_truncation_policy"
    Source = Join-Path $repoRoot "tests\replay\test_truncation_policy.cpp"
    ExtraObjects = @(
      (Join-Path $srcDir "build\sdl\replay_compare.o"),
      (Join-Path $srcDir "build\sdl\replay_compare_result.o"),
      (Join-Path $srcDir "build\sdl\replay_compare_diff.o"),
      (Join-Path $srcDir "build\sdl\replay_validate.o"),
      (Join-Path $srcDir "build\sdl\replay_contract.o")
    )
    Args = @()
  },
  @{
    Name = "test_scope_baseline_singleplayer"
    Source = Join-Path $repoRoot "tests\replay\test_scope_baseline_singleplayer.cpp"
    ExtraObjects = @(
      (Join-Path $srcDir "build\sdl\replay_verify.o")
    )
    Args = @()
  }
)

Set-Content -Path (Join-Path $buildDir "same-left.txt") -Value "same-state"
Set-Content -Path (Join-Path $buildDir "same-right.txt") -Value "same-state"

$passed = 0
$failed = 0

foreach ($test in $testCases) {
  $objPath = Join-Path $objDir ("{0}.o" -f $test.Name)
  $exePath = Join-Path $binDir ("{0}.exe" -f $test.Name)

  & g++ -Wall -W -Wno-deprecated-declarations -c $test.Source -o $objPath
  if ($LASTEXITCODE -ne 0) {
    Write-Host ("[FAIL] {0} (compile)" -f $test.Name)
    $failed++
    continue
  }

  $linkArgs = @("-o", $exePath, $objPath) + $test.ExtraObjects
  & g++ @linkArgs
  if ($LASTEXITCODE -ne 0) {
    Write-Host ("[FAIL] {0} (link)" -f $test.Name)
    $failed++
    continue
  }

  & $exePath @($test.Args)
  if ($LASTEXITCODE -eq 0) {
    Write-Host ("[PASS] {0}" -f $test.Name)
    $passed++
  }
  else {
    Write-Host ("[FAIL] {0}" -f $test.Name)
    $failed++
  }
}

Write-Host ("Replay tests complete: {0} passed, {1} failed." -f $passed, $failed)
if ($failed -gt 0) {
  exit 1
}
exit 0
