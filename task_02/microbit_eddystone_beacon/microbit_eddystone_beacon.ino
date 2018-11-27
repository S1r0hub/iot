// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


// FOR H AND CPP FILES SEE:
// https://github.com/sandeepmistry/arduino-BLEPeripheral/tree/master/src


// Import libraries (EddystoneBeacon depends on SPI)
#include <SPI.h>
#include <EddystoneBeacon.h>

// see here for code: https://github.com/adafruit/Adafruit_Microbit/blob/master/Adafruit_Microbit.h
#include <Adafruit_Microbit.h>
Adafruit_Microbit microbit;

const int buttonA = 5;
const int buttonB = 11;

// For Accelerometer: https://learn.adafruit.com/user-micro-bit-with-arduino/accelerometer-and-magnetometer
// Library download: https://cdn-learn.adafruit.com/assets/assets/000/046/217/original/MMA8653.zip
#include "Wire.h"
#include "MMA8653.h"
MMA8653 accel;

// define pins (varies per shield/board)
//
//   Adafruit Bluefruit LE   10, 2, 9
//   Blend                    9, 8, UNUSED
//   Blend Micro              6, 7, 4
//   RBL BLE Shield           9, 8, UNUSED

#define EDDYSTONE_BEACON_REQ   6
#define EDDYSTONE_BEACON_RDY   7
#define EDDYSTONE_BEACON_RST   4

EddystoneBeacon eddystoneBeacon = EddystoneBeacon(EDDYSTONE_BEACON_REQ, EDDYSTONE_BEACON_RDY, EDDYSTONE_BEACON_RST);
int i = 0;

// when to broadcast and how long
bool broadcasting = false;
const float broadcastTime = 10; // in seconds
unsigned long broadcasting_start = 0;

// time in ms for acceleration measurement
const int diffTime = 60;
float valx_p = 0;
float valy_p = 0;
float valz_p = 0;

// stores changes of acceleration
byte shakeByte = B00000000;

// last time a shake occured
unsigned long lastShakeTime = 0;
uint8_t shakesDone = 0;

// max amount of time in ms to wait between shakes
const float maxShakeWait = 1200;
const uint8_t shakesToSend = 2;

// rock, paper, scissor selection
uint8_t selection = 0;
const uint8_t selections = 3;
bool buttonA_pressed = false;
bool buttonB_pressed = false;

const char* rps_names[] = {
  "Rock",
  "Paper",
  "Scissor"
};

const uint8_t rps[3][5] = {
  {
    B11100,
    B10010,
    B11100,
    B10100,
    B10010
  },
  {
    B11100,
    B10010,
    B11100,
    B10000,
    B10000
  },
  {
    B01110,
    B10000,
    B01100,
    B00010,
    B11100
  }
};

const uint8_t check[] = {
  B00001,
  B00011,
  B10011,
  B11110,
  B01100
};


void setup() {
  Serial.begin(115200);
  delay(1000);

  // power = broadcasting power / transmit power
  // (usually between -40 and +4 dBm)
  eddystoneBeacon.begin(-18, ""); // power, URI
  Serial.println(F("Eddystone URL Beacon"));
  
  // Start LED matrix driver after radio (required)
  microbit.begin();

  // show initial selection on matrix display
  showSelection();

  // register micro bit buttons
  pinMode(buttonA, INPUT);
  pinMode(buttonB, INPUT);

  // 8-bit mode, 2g range
  accel.begin(false, 2);
}


void loop() {

  // show rock, paper or scissor
  if (!broadcasting) {

    // handle button press and show selection
    handleButtons();

    // check if user shaked the micro bit
    checkForShake();

    // check if user did enough shakes and send
    if (shakesDone >= shakesToSend) {
      shakesDone = 0;
      broadcasting = true;
      broadcasting_start = millis();

      eddystoneBeacon.setURI(rps_names[selection]);
      //eddystoneBeacon.begin(-18, rps_names[selection]); // power, URI
      Serial.print("Eddystone Beacon begin with: ");
      Serial.println(rps_names[selection]);
      
      microbit.matrix.show(check);
      delay(100);
    }
  }

  // directly enter the beacon loop
  if (broadcasting) {
    unsigned long curTime = millis();

    // end broadcast
    if (curTime > broadcasting_start + broadcastTime * 1000) {
      broadcasting = false;
      //eddystoneBeacon.end(); // calling end will not allow to start it again... :(
      eddystoneBeacon.setURI("");
      Serial.println("Broadcast end!");
      showSelection();
    }
    else {
      eddystoneBeacon.loop();
    }
  }
}


// read button inputs and set variables accordingly
void handleButtons() {

  bool aPressed = !digitalRead(buttonA);
  bool bPressed = !digitalRead(buttonB);

  if (aPressed && !buttonA_pressed) { buttonA_pressed = true; buttonAClick(); }
  else if (!aPressed && buttonA_pressed) { buttonA_pressed = false; }

  if (bPressed && !buttonB_pressed) { buttonB_pressed = true; buttonBClick(); }
  else if (!bPressed && buttonB_pressed) { buttonB_pressed = false; }
}


// handle clicks of button A
void buttonAClick() {
  changeSelection(-1);
}


// handle clicks of button B
void buttonBClick() {
  changeSelection(1);
}


// change r, p, s selection accordingly
void changeSelection(int it) {

  int nextSelection = selection + it;
  if (nextSelection < 0) { nextSelection = selections-1; }
  else if (nextSelection > selections-1) { nextSelection = 0; }

  // apply new selection
  selection = (uint8_t) nextSelection;
  Serial.print("Selection changed to: "); Serial.println(selection);
  
  // show selection on matrix
  showSelection();
}


// shows selection on matrix
void showSelection() {
  microbit.matrix.show(rps[selection]);
}


// shows the shakes as well
void showShakesDone() {
  for (uint i = 0; i < 5; i++) {
    uint16_t col = LED_OFF;
    if (shakesDone > i) { col = LED_ON; }
    microbit.matrix.drawPixel(4, 4-i, col);
  }
}


// Check if the user shaked the device.
// A shake will trigger broadcasting the selection
// for the specified amount of time.
void checkForShake() {
  
  // use accelerometer
  accel.update();
  float valx = accel.getX();
  float valy = accel.getY();
  float valz = accel.getZ();

  // calculate acceleration
  float accel_speed = abs(valx + valy + valz - valx_p - valy_p - valz_p) / float(diffTime);
  //Serial.print("accel_speed: "); Serial.println(accel_speed);

  // "free" space for the next bit
  shakeByte = shakeByte >> 1;

  // store 1 as bit at most left byte position (128 = 10000000) if threshold is exceeded
  if (accel_speed > 0.65) {
    shakeByte = shakeByte | 128;
  }
  
  //Serial.println(shakeByte, BIN);
  bool gotUp = false;
  bool gotBreak = false;
  bool gotDown = false;
  int breakTime = 0;
  for (int i = 0; i < 8; i++) {
    bool thisVal = shakeByte >> i & 1;
    //Serial.print(thisVal);
    //Serial.print(", ");

    if (thisVal) {
      if (!gotUp) {
        gotUp = true;
        //Serial.println("Got up");
      }
      
      if (gotBreak && !gotDown) {
        gotDown = true;
        //Serial.println("Got down");
      }
    }
    else {
      if (gotUp && !gotBreak) {
        breakTime++;
        //Serial.print("Break: "); Serial.println(breakTime);
        if (breakTime > 3) {
          gotBreak = true;
          //Serial.println("Got Break!");
        }
      }
    }
  }
  //Serial.println();

  if (gotUp && gotBreak && gotDown) {
    Serial.println("Shake!");

    unsigned long curTime = millis();
    if (curTime > lastShakeTime + maxShakeWait) {
      shakesDone = 1;
    }
    else {
      shakesDone += 1;
    }

    // print shake info
    Serial.print("Shakes done: ");
    Serial.print(shakesDone);
    Serial.print("/");
    Serial.print(shakesToSend);
    Serial.print(" (Time diff: ");
    Serial.print((curTime - lastShakeTime) / 1000.0);
    Serial.println(" seconds)");

    // store last shake time
    lastShakeTime = curTime;

    // reset the byte data
    shakeByte = 0;
  }

  valx_p = valx;
  valy_p = valy;
  valz_p = valz;

  delay(diffTime);

  // update matrix depending on shakes done
  //showShakesDone();
}
