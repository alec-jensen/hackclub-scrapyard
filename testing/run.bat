:: filepath: /c:/Users/brent/Documents/Programming Projects/HackClub/testing/run.bat
@echo off
setlocal EnableDelayedExpansion

:: Enable ANSI escape sequences
reg query HKCU\Console /v VirtualTerminalLevel >nul 2>&1 && goto :init
reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 1 /f >nul 2>&1

:init
:: Color definitions
set "GREEN=[32m"
set "YELLOW=[33m"
set "RED=[31m"
set "NC=[0m"

:: Get current time
for /f "tokens=1-3 delims=:." %%a in ("%TIME%") do (
    set "hour=%%a"
    set "minute=%%b"
    set "second=%%c"
)
set "hour=%hour: =%"

:: Logging functions
:log_main
echo %GREEN%[%hour%.%minute%.%second%] [MAIN]: %~1%NC%
goto :eof

:log_warning
echo %YELLOW%[%hour%.%minute%.%second%] [WARNING]: %~1%NC%
goto :eof

:log_error
echo %RED%[%hour%.%minute%.%second%] [ERROR]: %~1%NC%
goto :eof

:main
cls
call :log_main "Starting application..."

:: Check if executable exists
if not exist main.exe (
    call :log_error "main.exe not found! Please build the project first."
    pause
    exit /b 1
)

:: Check if application is already running and kill it
tasklist /FI "IMAGENAME eq main.exe" 2>NUL | find /I /N "main.exe">NUL
if not errorlevel 1 (
    call :log_warning "Application is already running. Closing previous instance..."
    taskkill /F /IM main.exe >NUL 2>&1
    timeout /t 1 >nul
)

:: Run the application with debug output
call :log_main "Launching main.exe..."
start "" main.exe
if errorlevel 1 (
    call :log_error "Failed to launch application! Error code: !errorlevel!"
    pause
    exit /b 1
)

:: Wait for application to start
timeout /t 2 >nul

:: Monitor the application
:loop
tasklist /FI "IMAGENAME eq main.exe" 2>NUL | find /I /N "main.exe">NUL
if errorlevel 1 (
    call :log_warning "Application has stopped running."
    goto :end
)
call :log_main "Application is running... Press Ctrl+C to exit"
timeout /t 2 >nul
goto :loop

:end
call :log_main "Cleaning up..."
taskkill /F /IM main.exe >NUL 2>&1
pause
exit /b 0

goto :main