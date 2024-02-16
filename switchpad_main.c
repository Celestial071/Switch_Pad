/* Serial com with arduino and C using Win32 api */
#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#define BIT1 0b10000000
#define BIT2 0b01000000
#define BIT3 0b00100000
#define BIT4 0b00010000
#define BIT5 0b00001000
#define BIT6 0b00000100
#define BIT7 0b00000010
#define BIT8 0b00000001

bool tryHandshake(HANDLE hSerial) {
    // Clear any previous data
    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    // Wait for the handshake message from Arduino
    DWORD bytesRead;
    char buffer[32];
    // Read may return immediately if there's no data, due to the setup of COMMTIMEOUTS
    BOOL readSuccess = ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    if (!readSuccess || bytesRead == 0) {
        return false; // Handshake failed, no data read
    }
    buffer[bytesRead] = '\0'; // Null-terminate the string

    // Check for handshake message
    if (strstr(buffer, "HANDSHAKE") != NULL) {
        // If handshake received, send ACK back to Arduino
        const char* ackMsg = "ACK\n";
        DWORD bytesWritten;
        WriteFile(hSerial, ackMsg, strlen(ackMsg), &bytesWritten, NULL);
        return true;
    }
    return false; // Handshake failed, incorrect data
}

HANDLE autoDetectArduinoPort() {
    HANDLE hSerial = INVALID_HANDLE_VALUE;
    for (int i = 1; i <= 256; i++) {
        char portName[20];
        sprintf(portName, "\\\\.\\COM%d", i);
        printf("Testing %s\n", portName);

        hSerial = CreateFileA(portName,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
        if (hSerial == INVALID_HANDLE_VALUE) {
            continue; // Unable to open port, try next
        }

        // Set up the device control block (DCB) for the serial port
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(DCB);
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            continue; // Unable to get the comm state, try next
        }

        // Configure the serial port settings
        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        if (!SetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            continue; // Unable to set the comm state, try next
        }

        // Set the timeouts for the serial port
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = 5000; // Set to 5 seconds
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 5000; // Also 5 seconds for writes
        timeouts.WriteTotalTimeoutMultiplier = 0;
        if (!SetCommTimeouts(hSerial, &timeouts)) {
            CloseHandle(hSerial);
            continue; // Unable to set timeouts, try next
        }

        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
        Sleep(2000); // Give Arduino time to reset

        if (tryHandshake(hSerial)) {
            printf("Arduino detected on %s\n", portName);
            return hSerial;
        } else {
            printf("Handshake failed on %s\n", portName);
            CloseHandle(hSerial); // Handshake failed, close and try next
        }
    }
    return INVALID_HANDLE_VALUE; // Arduino not found
}

void PressKey(WORD key){
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = key;
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));

    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}

void printBinary(uint8_t num) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (num >> i) & 1);
    }
    puts("");
}

void analyze_mouse(uint8_t d){
    POINT p;
    GetCursorPos(&p);
    int8_t x_offset = 0;
    int8_t y_offset = 0;
    if(d&BIT4) y_offset -= 7;
    if(d&BIT5) y_offset += 7;
    if(d&BIT6) x_offset += 12;
    if(d&BIT7) x_offset -= 12;

    //combining all
    SetCursorPos(p.x+x_offset, p.y + y_offset);

    //lets assume 'a' first bit is left mouse button and 'c' is right mouse button
    if(d&BIT1){
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));

        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    if(d&BIT3){
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &input, sizeof(INPUT));

        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &input, sizeof(INPUT));
    }
    //we aren't doing anything with the 2nd BIT rn :(
}

void analyze_joystick(uint8_t d){
    if(d&BIT2) PressKey(VK_SPACE);
    if(d&BIT2) PressKey(VK_RETURN);
    if(d&BIT3) PressKey('S');
    if(d&BIT4) PressKey(VK_UP);
    if(d&BIT5) PressKey(VK_DOWN);
    if(d&BIT6) PressKey(VK_LEFT);
    if(d&BIT7) PressKey (VK_RIGHT);
}
bool check = false;
int main(){
    //configure COM_Port
    /*char *portName = "\\\\.\\COM8"; //currently in COM8 for testing purpose
    HANDLE hSerial;

    hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    //error handling
    if(hSerial == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            //serial port does not exist
            printf("Serial Port %s not found!\n", portName);
        }
        printf("Error opening port %s\n", portName);
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if(!GetCommState(hSerial, &dcbSerialParams)){
        //error getting current serial params
        printf("Error getting serial port State\n");
        return -1;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if(!SetCommState(hSerial, &dcbSerialParams)){
        printf("Error setting serial com because the parameter is wrong (arduino and program aren't synced)\n");
        return 1;
    }

    //setting timeouts what are those?
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if(!SetCommTimeouts(hSerial, &timeouts)){
        printf("Error setting serial port timeouts.."); //ok but what does a timeout do?
        return 1;
    }

    uint8_t data = 0;
    DWORD dwBytesRead = 0;
    while(1){
        if(!ReadFile(hSerial, &data, sizeof(uint8_t), &dwBytesRead, NULL)){
            printf("Error Reading data from serial port\n");
            return 1;
        }
        if(dwBytesRead > 0)
            printf("Read %d bytes: %u\n", dwBytesRead, data);
        else
            Sleep(100);
        //trying to decode the information bits that was send...
        //since data is already encoded in the arduino we can just check for it and be happy
        toggle = data & BIT8;
        if(toggle)  analyze_mouse(data);
        else    analyze_joystick(data);
        data |= toggle;
        Sleep(100);
    }
    //close serial port
    CloseHandle(hSerial);*/

    //new implementation
    HANDLE hSerial = INVALID_HANDLE_VALUE;
    uint8_t toggle = 0x0;
    uint8_t data = 0x0;
    uint8_t prevData = 0xFF;
    // Initial connection
    hSerial = autoDetectArduinoPort();
    while (hSerial == INVALID_HANDLE_VALUE) {
        printf("Arduino not found. Retrying...\n");
        Sleep(5000); // Wait for 5 seconds before retrying
        hSerial = autoDetectArduinoPort();
    }
    printf("Communication with Arduino established.\n");

    while (1) {
        // Check connection and read data from Arduino
        DWORD bytesRead;
        if (!ReadFile(hSerial, &data, sizeof(data), &bytesRead, NULL) || bytesRead == 0) {
            printf("Arduino connection lost. Attempting to reconnect...\n");
            CloseHandle(hSerial);
            hSerial = autoDetectArduinoPort();
            while (hSerial == INVALID_HANDLE_VALUE) {
                printf("Arduino not found. Retrying...\n");
                Sleep(3000);
                hSerial = autoDetectArduinoPort();
            }
            printf("Communication with Arduino re-established.\n");
            continue;
        }else{
            if(data != prevData){
                toggle = data & BIT8;
                if(toggle){
                    analyze_mouse(data);
                }else   analyze_joystick(data);
                printBinary(data);
                prevData = data;
            }
        }
        Sleep(100);
   }
    CloseHandle(hSerial);
    return 0;
}
