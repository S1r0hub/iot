#include <Servo.h>

// Use for ESP 32 board:
// https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json

// Servo Library Download:
// https://github.com/RoboticsBrno/ESP32-Arduino-Servo-Library/archive/master.zip
// Installation:
//   1. Unpack master.zip
//   2. Get folder that is inside it and compress it to: ESP32_Arduino_Servo_Library.zip
//   3. Sketch -> Add Library (.zip) -> select this compressed file

Servo servo;
int pos = 0;
const int PIN_SERVO = 13;

void setup() {
  Serial.begin(115200);
  servo.attach(PIN_SERVO);
  Serial.println("Servo Attached.");
}

void loop() {

  for (pos = 0; pos < 180; pos++) {
    servo.write(pos);
    delay(10);
  }

  for (pos = 180; pos >= 0; pos--) {
    servo.write(pos);
    delay(10);
  }
  
}
