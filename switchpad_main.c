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
        dcbSerialParams.BaudRate = CBR_115200;
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
        timeouts.ReadTotalTimeoutConstant = 2000; 
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 2000;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        if (!SetCommTimeouts(hSerial, &timeouts)) {
            CloseHandle(hSerial);
            continue; // Unable to set timeouts, try next
        }

        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
        Sleep(3000); // Give Arduino time to reset

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
    if(d&BIT4) x_offset += 7;
    if(d&BIT5) x_offset -= 7;
    if(d&BIT6) y_offset -= 12;
    if(d&BIT7) y_offset += 12;

    //combining all
    SetCursorPos(p.x+x_offset, p.y + y_offset);

    //lets assume 'a' first bit is left mouse button and 'c' is right mouse button
    if(d&BIT2){
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
    if(d&BIT1) PressKey(VK_RETURN);
    if(d&BIT2) PressKey('S');
    if(d&BIT3) PressKey(VK_SPACE);
    if(d&BIT4) PressKey(VK_RIGHT);
    if(d&BIT5) PressKey(VK_LEFT);
    if(d&BIT6) PressKey(VK_UP);
    if(d&BIT7) PressKey (VK_DOWN);
}

bool check = false;
int main(){
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
    /*COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = MAXDWORD; // Immediate return
    timeouts.ReadTotalTimeoutConstant = 0;   // No additional wait time
    timeouts.ReadTotalTimeoutMultiplier = 0; // No multiplier
    if(!SetCommTimeouts(hSerial, &timeouts)){
        printf("Error setting timeouts.\n");
        CloseHandle(hSerial);
        return -1;
    }*/
    while (1) {
        // Check connection and read data from Arduino
        DWORD bytesRead;
        if (!ReadFile(hSerial, &data, sizeof(data), &bytesRead, NULL)) {
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
        }/*else{
            if(bytesRead > 0 && data != prevData){
                //means new data different than prev has been read
                prevData = data;
            }
        }*/
        //all thats needed now is for the program to keep on executing the code whenever state isn't changed but also execute it once when state is changed and if the state is still not changed then that means it will keep on excuting the code
        //why is this so slowww aaaaaaaaaaaaaaaaaaaaaaaaaaaa
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = MAXDWORD; // Immediate return
        timeouts.ReadTotalTimeoutConstant = 0;   // No additional wait time
        timeouts.ReadTotalTimeoutMultiplier = 0; // No multiplier
        if(!SetCommTimeouts(hSerial, &timeouts)){
            printf("Error setting timeouts.\n");
            CloseHandle(hSerial);
            return -1;
        }
        //need to make sure buttons aren't triggered alot of times and that will be the most beautiful addition to make this almost perfect

        toggle = data & BIT8;
        toggle?analyze_mouse(data):analyze_joystick(data);
        printBinary(data);
        Sleep(10);

   }
    CloseHandle(hSerial);
    return 0;
}
