@echo off
setlocal EnableDelayedExpansion

color 07
set "GREEN_TEXT=color 0A"
set "YELLOW_TEXT=color 0E"
set "RED_TEXT=color 0C"
set "NORMAL_TEXT=color 07"

for /f "tokens=1-3 delims=:." %%a in ("%TIME%") do (
    set "hour=%%a"
    set "minute=%%b"
    set "second=%%c"
)
set "hour=%hour: =%"

%YELLOW_TEXT%
echo [%hour%.%minute%.%second%] [WARNING]: Attempting to terminate main.exe...
%NORMAL_TEXT%

tasklist /FI "IMAGENAME eq main.exe" 2>NUL | find /I /N "main.exe">NUL
if !errorlevel! equ 0 (
    taskkill /F /IM main.exe >nul 2>&1
    if !errorlevel! equ 0 (
        %GREEN_TEXT%
        echo [%hour%.%minute%.%second%] [MAIN]: Successfully terminated main.exe
        %NORMAL_TEXT%
        exit /b 0
    ) else (
        %RED_TEXT%
        echo [%hour%.%minute%.%second%] [ERROR]: Failed to terminate main.exe
        %NORMAL_TEXT%
        exit /b 1
    )
) else (
    %YELLOW_TEXT%
    echo [%hour%.%minute%.%second%] [WARNING]: No running instances of main.exe found
    %NORMAL_TEXT%
    exit /b 0
)