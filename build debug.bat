@echo off
setlocal

set "UC_PROJECT=%~dp0uc_offline.vcxproj"
set "FS_PROJECT=%~dp0FakeSteamClient.vcxproj"

REM Find MSBuild
set "MSBUILD="
for /f "delims=" %%i in ('dir /b /s "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" 2^>nul') do (
    if not defined MSBUILD set "MSBUILD=%%i"
)

if not defined MSBUILD (
    echo [ERROR] MSBuild not found.
    echo.
    echo You need Visual Studio Build Tools 2022 or later.
    echo Install it here: https://visualstudio.microsoft.com/visual-cpp-build-tools/
    echo.
    echo Select "Desktop development with C++" during installation.
    echo.
    pause
    exit /b 1
)

echo Found MSBuild: %MSBUILD%
echo.

echo ========================================
echo  Building uc_offline x86 (debug) (steam_api.dll)
echo ========================================
"%MSBUILD%" "%UC_PROJECT%" -p:Configuration=Debug -p:Platform=Win32 -m
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] x86 uc_offline debug build failed.
    pause
    exit /b 1
)
echo x86 uc_offline debug build succeeded: debbuild\x86\steam_api.dll
echo.

echo ========================================
echo  Building uc_offline x64 (debug) (steam_api64.dll)
echo ========================================
"%MSBUILD%" "%UC_PROJECT%" -p:Configuration=Debug -p:Platform=x64 -m
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] x64 uc_offline debug build failed.
    pause
    exit /b 1
)
echo x64 uc_offline debug build succeeded: debbuild\x64\steam_api64.dll
echo.

echo ========================================
echo  Building FakeSteamClient x86 (debug) (steamclient.dll)
echo ========================================
"%MSBUILD%" "%FS_PROJECT%" -p:Configuration=Debug -p:Platform=Win32 -m
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] x86 FakeSteamClient debug build failed.
    pause
    exit /b 1
)
echo x86 FakeSteamClient debug build succeeded: debbuild\x86\steamclient.dll
echo.

echo ========================================
echo  Building FakeSteamClient x64 (debug) (steamclient64.dll)
echo ========================================
"%MSBUILD%" "%FS_PROJECT%" -p:Configuration=Debug -p:Platform=x64 -m
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] x64 FakeSteamClient debug build failed.
    pause
    exit /b 1
)
echo x64 FakeSteamClient debug build succeeded: debbuild\x64\steamclient64.dll
echo.

echo ========================================
echo  All debug builds completed successfully
echo ========================================
echo  x86: debbuild\x86\steam_api.dll / steamclient.dll
echo  x64: debbuild\x64\steam_api64.dll / steamclient64.dll
echo ========================================

pause
