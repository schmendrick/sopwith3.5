@echo off
setlocal EnableExtensions
set "MSYS2=C:\msys64"
if defined MSYS2_ROOT set "MSYS2=%MSYS2_ROOT%"

set "OP=all"
if /i "%~1"=="clean" set "OP=clean"

set "BATDIR=%~dp0"
if "%BATDIR:~-1%"=="\" set "BATDIR=%BATDIR:~0,-1%"

set "SRC=%BATDIR%\src"
set "SRCBASH=%SRC:\=/%"

if not exist "%MSYS2%\usr\bin\env.exe" (
  echo build.bat: MSYS2 not found at "%MSYS2%". Install MSYS2 or set MSYS2_ROOT to your install directory.
  exit /b 1
)

if not exist "%SRC%\Makefile.msys2" (
  echo build.bat: missing "%SRC%\Makefile.msys2"
  exit /b 1
)

"%MSYS2%\usr\bin\env.exe" MSYSTEM=MINGW64 /usr/bin/bash -lc "cd '%SRCBASH%' && mingw32-make -f Makefile.msys2 %OP%"
exit /b %ERRORLEVEL%
