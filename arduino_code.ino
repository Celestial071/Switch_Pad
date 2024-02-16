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
bool check = false; // Used for toggling BIT8

void setup() {
  Serial.begin(9600);
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
    if (digitalRead(button[0])) data |= BIT1;
    if (digitalRead(button[2])) data |= BIT2;
    if (digitalRead(button[3])) data |= BIT3;
    if (analogRead(A1) < 250) data |= BIT5;
    if (analogRead(A1) > 750) data |= BIT4;
    if (analogRead(A0) < 250) data |= BIT7;
    if (analogRead(A0) > 750) data |= BIT6;

    if (digitalRead(button[1])) {
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
