#!/bin/bash

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_main() {
    echo -e "${GREEN}[$(date +%H.%M.%S)] [MAIN]: $1${NC}"
}

log_warning() {
    echo -e "${YELLOW}[$(date +%H.%M.%S)] [WARNING]: $1${NC}"
}

log_error() {
    echo -e "${RED}[$(date +%H.%M.%S)] [ERROR]: $1${NC}"
}

# Clear screen
clear

log_main "Starting build process with MinGW in WSL..."

# Check if required tools are available
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    log_error "MinGW compiler not found! Please install mingw-w64"
    exit 1
fi

# Check if source files exist
if [ ! -f main.cpp ] || [ ! -f middleWhere.cpp ]; then
    log_error "Required source files not found!"
    exit 1
fi

# Terminate existing process if running
log_warning "Attempting to terminate any running instances of main.exe"
cmd.exe /c kill.bat || {
    log_error "Failed to execute kill.bat"
    exit 1
}

# Remove existing executable
if [ -f main.exe ]; then
    log_warning "Removing existing main.exe"
    rm -f main.exe 2>/dev/null || {
        log_error "Failed to remove existing main.exe - file may be in use"
        exit 1
    }
fi
# Create output directory if it doesn't exist
OUTPUT_DIR="build"
mkdir -p "$OUTPUT_DIR" 2>/dev/null || {
    log_error "Failed to create build directory"
    exit 1
}

# Build the project
log_main "Compiling project..."
x86_64-w64-mingw32-g++ main.cpp middleWhere.cpp -o "$OUTPUT_DIR/main.exe" \
    -I. \
    -luser32 -lgdi32 \
    -static-libgcc -static-libstdc++ \
    -D_WIN32_WINNT=0x0601 \
    -DENABLE_TAK_SKILL \
    -Wall -Wextra -Wpedantic \
    -O2

if [ $? -eq 0 ]; then
    log_main "Build completed successfully!"
    
    # Check executable size
    if [ -f "$OUTPUT_DIR/main.exe" ]; then
        size=$(stat -f %z "$OUTPUT_DIR/main.exe" 2>/dev/null || stat -c %s "$OUTPUT_DIR/main.exe" 2>/dev/null)
        log_main "Executable size: $(($size/1024)) KB"
        
        # Copy to final location
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