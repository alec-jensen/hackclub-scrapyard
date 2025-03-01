@echo off
setlocal EnableDelayedExpansion

:: Color definitions
set "GREEN=[32m"
set "YELLOW=[33m"
set "RED=[31m"
set "NC=[0m"

:: Get current time for logging
for /f "tokens=1-3 delims=:." %%a in ("%TIME%") do (
    set "hour=%%a"
    set "minute=%%b"
    set "second=%%c"
)
set "hour=%hour: =%"

echo %YELLOW%[%hour%.%minute%.%second%] [WARNING]: Attempting to terminate main.exe...%NC%

:: Check if process exists first
tasklist /FI "IMAGENAME eq main.exe" 2>NUL | find /I /N "main.exe">NUL
if !errorlevel! equ 0 (
    taskkill /F /IM main.exe >nul 2>&1
    if !errorlevel! equ 0 (
        echo %GREEN%[%hour%.%minute%.%second%] [MAIN]: Successfully terminated main.exe%NC%
        exit /b 0
    ) else (
        echo %RED%[%hour%.%minute%.%second%] [ERROR]: Failed to terminate main.exe%NC%
        exit /b 1
    )
) else (
    echo %YELLOW%[%hour%.%minute%.%second%] [WARNING]: No running instances of main.exe found%NC%
    exit /b 0
)