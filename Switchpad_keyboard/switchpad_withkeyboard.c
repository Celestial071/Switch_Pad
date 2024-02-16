#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#define BIT1 0b10000000
#define BIT2 0b01000000
#define BIT3 0b00100000
#define BIT4 0b00010000
#define BIT5 0b00001000
#define BIT6 0b00000100
#define BIT7 0b00000010
#define BIT8 0b00000001

void printBinary(uint8_t num) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (num >> i) & 1);
    }
}

uint8_t data = 0x0;

void analyzerfunction_joystick(uint8_t d){
    if(d & BIT1)
        printf("space was pressed!\n");
    if(d & BIT2)
        printf("enter was pressed!\n");
    if(d & BIT3)
        printf("s was pressed!\n");
    if(d & BIT4)
        printf("up arrow key was pressed\n");
    if(d & BIT5)
        printf("down arrow key was pressed\n");
    if(d & BIT6)
        printf("right arrow key was pressed\n");
    if(d & BIT7)
        printf("left arrow key was pressed\n");
}
void analyzerfunction_mouse(uint8_t d){
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
}

bool check = false;
int main() {
    uint8_t toggle = 0x0;
    printf("\n");
    do{
        if(GetAsyncKeyState(' ') & 0x8000) data|=BIT1;
        if(GetAsyncKeyState(VK_RETURN) & 0x8000) data|=BIT2;
        if(GetAsyncKeyState('S') & 0x8000) data|=BIT3;
        if(GetAsyncKeyState(VK_UP) & 0x8000) data|=BIT4;
        if(GetAsyncKeyState(VK_DOWN) & 0x8000) data|=BIT5;
        if(GetAsyncKeyState(VK_RIGHT) & 0x8000) data|=BIT6;
        if(GetAsyncKeyState(VK_LEFT) & 0x8000) data|=BIT7;

        if(GetAsyncKeyState('T') & 0x8000){ 
            if(!check){
                data ^= BIT8;
                check = true;
            }else{
                check = false;
            }
        }

        toggle = data & BIT8;
        if(data & BIT8)
            analyzerfunction_mouse(data);
        else
            analyzerfunction_joystick(data);
        data = 0x0;
        data |= toggle;
        if(GetAsyncKeyState('Q') & 0x8000){
            printf("Breaking from the program\n");
            break;  
        }
        Sleep(10);  
    }while(1);
    printf("\n");
     
    return 0;
}
