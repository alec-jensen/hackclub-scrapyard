@echo off
setlocal EnableDelayedExpansion

reg query HKCU\Console /v VirtualTerminalLevel >nul 2>&1 || reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 1 /f >nul 2>&1

set "GREEN=[32m"
set "YELLOW=[33m"
set "RED=[31m"
set "NC=[0m"

for /f "tokens=1-3 delims=:." %%a in ("%TIME%") do (
    set "hour=%%a"
    set "minute=%%b"
    set "second=%%c"
)
set "hour=%hour: =%"

echo %GREEN%[%hour%.%minute%.%second%] [MAIN]: Starting application...%NC%


if not exist "build\main.exe" (
    echo %RED%[%hour%.%minute%.%second%] [ERROR]: main.exe not found in build directory!%NC%
    echo %YELLOW%[%hour%.%minute%.%second%] [WARNING]: Please run build.sh first.%NC%
    pause
    exit /b 1
)


tasklist /FI "IMAGENAME eq main.exe" 2>NUL | find /I /N "main.exe">NUL
if not errorlevel 1 (
    echo %YELLOW%[%hour%.%minute%.%second%] [WARNING]: Application is already running. Closing previous instance...%NC%
    taskkill /F /IM main.exe >NUL 2>&1
    timeout /t 1 >nul
)

echo %GREEN%[%hour%.%minute%.%second%] [MAIN]: Launching main.exe with console window...%NC%
start "Keyboard Middleware" cmd /k "cd /d %~dp0\build && main.exe"

echo %GREEN%[%hour%.%minute%.%second%] [MAIN]: Application launched in a separate window.%NC%
echo %YELLOW%[%hour%.%minute%.%second%] [WARNING]: Close that window to terminate the application.%NC%