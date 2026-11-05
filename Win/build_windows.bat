@echo off
REM ====================================================================
REM  Build DreamyMusic.aex for Windows (64-bit Adobe After Effects)
REM  Run from a "x64 Native Tools Command Prompt for VS 2022" or CMake
REM ====================================================================

setlocal
cd /d "%~dp0\.."

if not exist "build\Windows\Release" mkdir "build\Windows\Release"

where cl.exe >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [1/2] Compiling Windows PiPL Resource (DreamyMusicPiPL.rc)...
    rc.exe /nologo /fo "build\Windows\Release\DreamyMusicPiPL.res" "Win\DreamyMusicPiPL.rc"
    if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

    echo [2/2] Compiling and Linking DreamyMusic.aex (x64 Release)...
    cl.exe /nologo /std:c++17 /O2 /Oi /Ot /fp:fast /arch:AVX2 /EHsc /MT ^
        /DAE_OS_WIN=1 /DAE_PROC_INTELx64=1 /D_CRT_SECURE_NO_WARNINGS=1 /DNDEBUG=1 ^
        /I"src" /I"include\AE_SDK" ^
        "src\DreamyMusic.cpp" ^
        /link /DLL /DEF:"Win\DreamyMusic.def" ^
        /OUT:"build\Windows\Release\DreamyMusic.aex" ^
        "build\Windows\Release\DreamyMusicPiPL.res" kernel32.lib user32.lib
    if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

    echo.
    echo Build succeeded: build\Windows\Release\DreamyMusic.aex
    echo Copy DreamyMusic.aex to:
    echo   C:\Program Files\Adobe\Common\Plug-ins\7.0\MediaCore\
    exit /b 0
)

where cmake.exe >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Building with CMake...
    cmake -B build\Windows\cmake -DCMAKE_BUILD_TYPE=Release
    cmake --build build\Windows\cmake --config Release
    exit /b %ERRORLEVEL%
)

echo Error: Neither MSVC (cl.exe) nor CMake was found in PATH.
echo Please open "x64 Native Tools Command Prompt for VS" and run Win\build_windows.bat.
exit /b 1

