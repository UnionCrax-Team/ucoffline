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
echo  Building uc_offline x86 (steam_api.dll)
echo ========================================
"%MSBUILD%" "%UC_PROJECT%" -p:Configuration=Release -p:Platform=Win32 -m
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] x86 uc_offline build failed.
    pause
    exit /b 1
)
echo x86 uc_offline build succeeded: relbuild\x86\steam_api.dll
echo.

echo ========================================
echo  Building uc_offline x64 (steam_api64.dll)
echo ========================================
"%MSBUILD%" "%UC_PROJECT%" -p:Configuration=Release -p:Platform=x64 -m
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] x64 uc_offline build failed.
    pause
    exit /b 1
)
echo x64 uc_offline build succeeded: relbuild\x64\steam_api64.dll
echo.

echo ========================================
echo  Building FakeSteamClient x86 (steamclient.dll)
echo ========================================
"%MSBUILD%" "%FS_PROJECT%" -p:Configuration=Release -p:Platform=Win32 -m
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] x86 FakeSteamClient build failed.
    pause
    exit /b 1
)
echo x86 FakeSteamClient build succeeded: relbuild\x86\steamclient.dll
echo.

echo ========================================
echo  Building FakeSteamClient x64 (steamclient64.dll)
echo ========================================
"%MSBUILD%" "%FS_PROJECT%" -p:Configuration=Release -p:Platform=x64 -m
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] x64 FakeSteamClient build failed.
    pause
    exit /b 1
)
echo x64 FakeSteamClient build succeeded: relbuild\x64\steamclient64.dll
echo.

echo ========================================
echo  All builds completed successfully
echo ========================================
echo  x86: relbuild\x86\steam_api.dll / steamclient.dll
echo  x64: relbuild\x64\steam_api64.dll / steamclient64.dll
echo ========================================

pause
