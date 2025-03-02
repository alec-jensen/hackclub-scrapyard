#!/bin/bash

if [ -t 1 ]; then
    RED=$(tput setaf 1)
    GREEN=$(tput setaf 2)
    YELLOW=$(tput setaf 3)
    BOLD=$(tput bold)
    RESET=$(tput sgr0)
else
    RED='\033[1;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BOLD='\033[1m'
    RESET='\033[0m'
fi

log_main() {
    echo -e "${GREEN}${BOLD}[$(date +%H.%M.%S)] [MAIN]:${RESET} ${GREEN}$1${RESET}"
}

log_warning() {
    echo -e "${YELLOW}${BOLD}[$(date +%H.%M.%S)] [WARNING]:${RESET} ${YELLOW}$1${RESET}"
}

log_error() {
    echo -e "${RED}${BOLD}[$(date +%H.%M.%S)] [ERROR]:${RESET} ${RED}$1${RESET}"
}

if grep -q Microsoft /proc/version; then
    cmd.exe /c "reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 1 /f >nul 2>&1" >/dev/null 2>&1
fi

clear

log_main "Starting build process with MinGW in WSL..."

if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    log_error "MinGW compiler not found! Please install mingw-w64"
    exit 1
fi

if [ ! -f main.cpp ] || [ ! -f middleWhere.cpp ] || [ ! -f Logger.cpp ]; then
    log_error "Required source files not found!"
    exit 1
fi

log_warning "Attempting to terminate any running instances of main.exe"

kill_output=$(cmd.exe /c kill.bat 2>&1)
echo -e "$kill_output" | while read -r line; do
    if [[ $line == *"[WARNING]"* ]]; then
        echo -e "${YELLOW}${BOLD}$line${RESET}"
    elif [[ $line == *"[ERROR]"* ]]; then
        echo -e "${RED}${BOLD}$line${RESET}"
    elif [[ $line == *"[MAIN]"* ]]; then
        echo -e "${GREEN}${BOLD}$line${RESET}"
    else
        echo -e "$line"
    fi
done

if [ $? -ne 0 ]; then
    log_error "Failed to execute kill.bat"
    exit 1
fi

if [ -f main.exe ]; then
    log_warning "Removing existing main.exe"
    rm -f main.exe 2>/dev/null || {
        log_error "Failed to remove existing main.exe - file may be in use"
        exit 1
    }
fi
OUTPUT_DIR="build"
mkdir -p "$OUTPUT_DIR" 2>/dev/null || {
    log_error "Failed to create build directory"
    exit 1
}

log_main "Compiling project..."
x86_64-w64-mingw32-g++ main.cpp middleWhere.cpp Logger.cpp -o "$OUTPUT_DIR/main.exe" \
    -I. \
    -luser32 -lgdi32 \
    -static-libgcc -static-libstdc++ \
    -D_WIN32_WINNT=0x0601 \
    -DENABLE_TAK_SKILL \
    -Wall -Wextra -Wpedantic \
    -O2

if [ $? -eq 0 ]; then
    log_main "Build completed successfully!"
    
    if [ -f "$OUTPUT_DIR/main.exe" ]; then
        size=$(stat -f %z "$OUTPUT_DIR/main.exe" 2>/dev/null || stat -c %s "$OUTPUT_DIR/main.exe" 2>/dev/null)
        log_main "Executable size: $(($size/1024)) KB"
        
        cp "$OUTPUT_DIR/main.exe" ./main.exe 2>/dev/null || {
            log_error "Failed to copy executable to final location"
            exit 1
        }
    else
        log_error "Build succeeded but executable not found!"
        exit 1
    fi
else
    log_error "Build failed!"
    exit 1
fi