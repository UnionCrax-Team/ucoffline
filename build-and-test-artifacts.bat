@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "ROOT=%~dp0"
set "UC_PROJECT=%ROOT%uc_offline.vcxproj"
set "FS_PROJECT=%ROOT%FakeSteamClient.vcxproj"
set "CONFIG=%~1"

if /i "%CONFIG%"=="--help" goto :usage
if /i "%CONFIG%"=="-h" goto :usage
if "%CONFIG%"=="" set "CONFIG=Release"

if /i "%CONFIG%"=="Release" (
    set "BUILD_DIR=%ROOT%relbuild"
) else if /i "%CONFIG%"=="Debug" (
    set "BUILD_DIR=%ROOT%debbuild"
) else (
    echo [ERROR] Unknown configuration "%CONFIG%".
    goto :usage
)

set "LOGDIR=%ROOT%logs"
set "DIST=%ROOT%dist"
set "MSBUILD=%MSBUILD%"
if defined MSBUILD if not exist "%MSBUILD%" set "MSBUILD="

if not exist "%LOGDIR%" mkdir "%LOGDIR%"
if not exist "%DIST%" mkdir "%DIST%"

call :findMsBuild
if not defined MSBUILD (
    echo [ERROR] MSBuild was not found.
    echo Install Visual Studio 2022 Build Tools or Visual Studio 2022 with Desktop development with C++.
    exit /b 1
)

echo Found MSBuild: %MSBUILD%
call :findDumpbin
if not defined DUMPBIN (
    echo [WARN] dumpbin.exe was not found; skipping PE architecture and export checks.
)

call :buildOne Win32 x86 steam_api.dll "%UC_PROJECT%"
if errorlevel 1 exit /b 1

call :buildOne x64 x64 steam_api64.dll "%UC_PROJECT%"
if errorlevel 1 exit /b 1

call :buildOne Win32 x86 steamclient.dll "%FS_PROJECT%"
if errorlevel 1 exit /b 1

call :buildOne x64 x64 steamclient64.dll "%FS_PROJECT%"
if errorlevel 1 exit /b 1

call :verifyArtifact "%BUILD_DIR%\x86\steam_api.dll" x86
if errorlevel 1 exit /b 1

call :verifyArtifact "%BUILD_DIR%\x64\steam_api64.dll" x64
if errorlevel 1 exit /b 1

call :verifyArtifact "%BUILD_DIR%\x86\steamclient.dll" x86
if errorlevel 1 exit /b 1

call :verifyArtifact "%BUILD_DIR%\x64\steamclient64.dll" x64
if errorlevel 1 exit /b 1

call :stageArtifact "%BUILD_DIR%\x86\steam_api.dll" "%DIST%\x86"
if errorlevel 1 exit /b 1

call :stageArtifact "%BUILD_DIR%\x64\steam_api64.dll" "%DIST%\x64"
if errorlevel 1 exit /b 1

call :stageArtifact "%BUILD_DIR%\x86\steamclient.dll" "%DIST%\x86"
if errorlevel 1 exit /b 1

call :stageArtifact "%BUILD_DIR%\x64\steamclient64.dll" "%DIST%\x64"
if errorlevel 1 exit /b 1

echo.
echo All builds and artifact checks passed.
echo Staged artifacts:
echo   %DIST%\x86\steam_api.dll / steamclient.dll
echo   %DIST%\x64\steam_api64.dll / steamclient64.dll
exit /b 0

:usage
echo Usage: %~nx0 [Debug^|Release]
echo Defaults to Release.
echo Builds Win32 and x64, validates DLL artifacts, and stages them under dist\.
exit /b 0

:findMsBuild
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find "MSBuild\**\Bin\MSBuild.exe" 2^>nul`) do (
        if exist "%%i" if not defined MSBUILD set "MSBUILD=%%i"
    )
)
if defined MSBUILD exit /b 0

for %%i in (
    "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\18\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2026\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2026\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files ^(x86^)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files ^(x86^)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files ^(x86^)\Microsoft Visual Studio\2017\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
) do (
    if exist "%%~i" if not defined MSBUILD set "MSBUILD=%%~i"
)
exit /b 0

:findDumpbin
set "DUMPBIN="
for /f "delims=" %%i in ('where dumpbin 2^>nul') do (
    if not defined DUMPBIN set "DUMPBIN=%%i"
)
if defined DUMPBIN exit /b 0

if defined MSBUILD (
    for %%I in ("%MSBUILD%") do (
        set "VSDIR=%%~dpI..\..\.."
        for /d %%V in ("!VSDIR!\VC\Tools\MSVC\*") do (
            if exist "%%V\bin\HostX64\x64\dumpbin.exe" if not defined DUMPBIN set "DUMPBIN=%%V\bin\HostX64\x64\dumpbin.exe"
        )
    )
)
exit /b 0

:buildOne
set "PLATFORM=%~1"
set "ARCH=%~2"
set "ARTIFACT=%~3"
set "BUILD_PROJECT=%~4"
set "LOG=%LOGDIR%\build-%CONFIG%-%ARCH%-%ARTIFACT%.log"
set "BINLOG=%LOGDIR%\msbuild-%CONFIG%-%ARCH%-%ARTIFACT%.binlog"

echo.
echo Building %CONFIG% ^| %PLATFORM% ^| %ARTIFACT%
"%MSBUILD%" "%BUILD_PROJECT%" -p:Configuration=%CONFIG% -p:Platform=%PLATFORM% -m -v:m /bl:"%BINLOG%" > "%LOG%" 2>&1
if errorlevel 1 (
    echo [ERROR] %CONFIG% ^| %PLATFORM% ^| %ARTIFACT% build failed.
    echo See %LOG% and %BINLOG%.
    exit /b 1
)

echo Build succeeded: %BUILD_DIR%\%ARCH%\%ARTIFACT%
exit /b 0

:verifyArtifact
set "ARTIFACT=%~1"
set "MACHINE=%~2"
set "NAME=%~nx1"
set "HEADERS=%LOGDIR%\dumpbin-headers-%NAME%.txt"
set "EXPORTS=%LOGDIR%\dumpbin-exports-%NAME%.txt"

if not exist "%ARTIFACT%" (
    echo [ERROR] Missing artifact: %ARTIFACT%
    exit /b 1
)

for %%A in ("%ARTIFACT%") do set "SIZE=%%~zA"
if not defined SIZE (
    echo [ERROR] Could not read artifact size: %ARTIFACT%
    exit /b 1
)
if %SIZE% LEQ 0 (
    echo [ERROR] Artifact is empty: %ARTIFACT%
    exit /b 1
)

echo Artifact exists: %ARTIFACT% (%SIZE% bytes)

if not defined DUMPBIN exit /b 0

"%DUMPBIN%" /headers "%ARTIFACT%" > "%HEADERS%" 2>&1
if errorlevel 1 (
    echo [ERROR] dumpbin /headers failed for %ARTIFACT%.
    echo See %HEADERS%.
    exit /b 1
)

findstr /I /C:"machine (%MACHINE%)" "%HEADERS%" >nul
if errorlevel 1 (
    echo [ERROR] Artifact machine type does not match %MACHINE%: %ARTIFACT%
    echo See %HEADERS%.
    exit /b 1
)
echo Machine type verified: %MACHINE%

"%DUMPBIN%" /exports "%ARTIFACT%" > "%EXPORTS%" 2>&1
if errorlevel 1 (
    echo [ERROR] dumpbin /exports failed for %ARTIFACT%.
    echo See %EXPORTS%.
    exit /b 1
)

findstr /I /C:"number of functions" "%EXPORTS%" >nul
if errorlevel 1 (
    echo [ERROR] Export table was not found in %ARTIFACT%.
    echo See %EXPORTS%.
    exit /b 1
)

set "EXPORT_COUNT="
for /f "tokens=1" %%i in ('findstr /I /C:"number of functions" "%EXPORTS%"') do set "EXPORT_COUNT=%%i"
if not defined EXPORT_COUNT (
    echo [ERROR] Could not parse export count for %ARTIFACT%.
    echo See %EXPORTS%.
    exit /b 1
)
if %EXPORT_COUNT% LEQ 0 (
    echo [ERROR] Artifact has no exports: %ARTIFACT%
    exit /b 1
)
echo Export table verified: %EXPORT_COUNT% exports
exit /b 0

:stageArtifact
set "SRC=%~1"
set "DST_DIR=%~2"
set "DST_NAME=%~nx1"

if not exist "%DST_DIR%" mkdir "%DST_DIR%"
copy /Y "%SRC%" "%DST_DIR%\%DST_NAME%" >nul
if errorlevel 1 (
    echo [ERROR] Failed to stage artifact: %SRC%
    exit /b 1
)

echo Staged artifact: %DST_DIR%\%DST_NAME%
exit /b 0
