#include <stdint.h>

#define BIT1 0b10000000
#define BIT2 0b01000000
#define BIT3 0b00100000
#define BIT4 0b00010000
#define BIT5 0b00001000
#define BIT6 0b00000100
#define BIT7 0b00000010
#define BIT8 0b00000001

uint8_t data = 0x0;
uint8_t prevData = 0xFF;
uint8_t toggle = 0x0;
uint8_t button[] = {2, 3, 4, 5};
bool handshake_accept = false;
bool but1_down = false;
bool but2_down = false;
bool but3_down = false;
bool check = false; // Used for toggling BIT8

void setup() {
  Serial.begin(115200);
  delay(2000);
  // Initialize button pins as inputs
  for (int i = 0; i < 4; i++) {
    pinMode(button[i], INPUT);
  }
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
}

void loop() {
  if (!handshake_accept) {

    Serial.println("HANDSHAKE");
    while (Serial.available() == 0) {
      ; // Wait for response
    }
    String response = Serial.readStringUntil('\n');
    if (response.startsWith("ACK")) {
      handshake_accept = true;
    }
  } else {
    data = 0x0;
    if (digitalRead(button[0])){
      if(!but1_down){
        data |= BIT1;
        but1_down = true;
      }
    }else{
      but1_down = false;
      //might need to reset the bit on button 3
    }
    if (digitalRead(button[1])){
      if(!but2_down){
      data |= BIT2;
      but2_down = true;
      }
    }else{
      but2_down = false;
      //might need to reset the bit on button 2
    }
    if (digitalRead(button[2])){
      if(!but3_down){ 
        data |= BIT3;
        but3_down = true;
      }
    }else{
      but3_down = false;
      //might need to reset the bit of button3
    }
  
    if (analogRead(A1) < 250) data |= BIT5;
    if (analogRead(A1) > 750) data |= BIT4;
    if (analogRead(A0) < 250) data |= BIT7;
    if (analogRead(A0) > 750) data |= BIT6;

    if (digitalRead(button[3])) {
      if (!check) {
        toggle ^= BIT8; // Toggle the state of BIT8
        check = true;
      }
    } else {
      check = false;
    }

    data |= toggle;

    if (data != prevData) {
      Serial.write(data);
      prevData = data;
    }
  }
  delay(10);
}
