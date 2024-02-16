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
uint8_t button[] = {2,3,4,5};
bool handshake_accept = false;

/*void printBinary(uint8_t d){
  for (int i = 7; i >= 0; i--) {
        Serial.print((d >> i) & 1);
    }
    Serial.println("");
}*/
void setup() {
  Serial.begin(9600);
  

  // Wait a bit for the serial port to establish connection with the PC
  delay(2000);
}

void loop() {
  if(!handshake_accept){
    Serial.println("HANDSHAKE");  // Send the handshake message
    while(Serial.available() == 0) {
      ; // Do nothing until we get a response
    }
    String response = Serial.readStringUntil('\n'); // Read the response
    if(response.startsWith("ACK")) {
      Serial.println("Handshake received!");
      handshake_accept = true;
    }
  }else{
    //encoding the data in the format I want;;;
    if(digitalRead(button[0]))  data |= BIT1;
    if(digitalRead(button[1]))  data |= BIT8;
    if(digitalRead(button[2]))  data |= BIT2;
    if(digitalRead(button[3]))  data |= BIT3;

    if(analogRead(A1) < 250)  data |= BIT5;
    if(analogRead(A1) > 750)  data |= BIT4;
    if(analogRead(A0) < 250)  data |= BIT7;
    if(analogRead(A0) > 750)  data |= BIT6;

    Serial.write(data);
  }
  data = 0x0;
}
