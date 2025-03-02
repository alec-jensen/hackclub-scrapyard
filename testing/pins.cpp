#include <windows.h>
#include <iostream>
#include <string>

int main() {
    // Serial port parameters
    const wchar_t* portName = L"COM3";  // Changed to wide character string
    HANDLE serialHandle;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};
    
    // Open the serial port
    serialHandle = CreateFile(
        portName, 
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL
    );
    
    // Rest of the code remains the same...Plea