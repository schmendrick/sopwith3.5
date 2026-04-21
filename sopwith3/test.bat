@echo off
setlocal

pushd "%~dp0src" || exit /b 1
mingw32-make -f Makefile.msys2 replay-tests
set EXIT_CODE=%ERRORLEVEL%
popd

if not "%EXIT_CODE%"=="0" (
  echo Replay tests failed with exit code %EXIT_CODE%.
  exit /b %EXIT_CODE%
)

echo Replay tests passed.
exit /b 0
