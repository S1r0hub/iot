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

void setup() {
  Serial.begin(115200);
  delay(1000);

  // power = broadcasting power / transmit power
  // (usually between -40 and +4 dBm)
  eddystoneBeacon.begin(-20, "Initialized"); // power, URI
  Serial.println(F("Eddystone URL Beacon"));
  
  // Start LED matrix driver after radio (required)
  microbit.begin();

  // 8-bit mode, 2g range
  accel.begin(false, 2);
}

//int amin = 9999999;
//int amax = -9999999;

int diffTime = 50;
float valx_p = 0;
float valy_p = 0;
float valz_p = 0;

//float ms[10];
byte shakeByte = B00000000;

void loop() {

  /*
  i++;
  char* out = "Rock";
  if (i % 2 == 0) { 
    out = "The";
    microbit.matrix.print(out);
  }
  */
  
  //eddystoneBeacon.setURI(out);
  //eddystoneBeacon.loop();
  //delay(10);


  // accel test
  accel.update();
  float valx = accel.getX();
  float valy = accel.getY();
  float valz = accel.getZ();

  //if (val < amin) { amin = val; }
  //if (val > amax) { amax = val; }
  
  //Serial.print("Accel Z: ");
  //Serial.println(val);
  //Serial.print("Min: "); Serial.print(amin); Serial.print(", Max: "); Serial.print(amax); Serial.print(", Cur: "); Serial.println(val);

  float accel_speed = abs(valx + valy + valz - valx_p - valy_p - valz_p) / float(diffTime);

  //Serial.print("accel_speed: "); Serial.println(accel_speed);

  /*
  for (int i = 9; i > 0; i--) {
    ms[i] = ms[i-1];
  }
  ms[0] = accel_speed;

  for (int i = 0; i < 10; i++) {
    Serial.print(ms[i] > 1.0);
    Serial.print(", ");
  }
  Serial.println();
  */

  shakeByte = shakeByte >> 1;

  // store 1 as bit at most left byte position (128 = 10000000)
  // if the threshold is reached
  if (accel_speed > 0.7) {
    shakeByte = shakeByte | 128;
  }
  
  //Serial.println(shakeByte, BIN);
  bool gotUp = false;
  bool gotBreak = false;
  bool gotDown = false;
  int breakTime = 0;
  for (int i = 0; i < 8; i++) {
    bool thisVal = shakeByte >> i & 1;
    Serial.print(thisVal);
    Serial.print(", ");

    if (thisVal) {
      if (!gotUp) {
        gotUp = true; 
      }
      
      if (gotBreak && !gotDown) {
        gotDown = true;
      }
    }
    else {
      if (gotUp && !gotBreak) {
        breakTime++;
        if (breakTime > 4); {
          gotBreak = true;
        }
      }
    }
  }
  Serial.println();

  if (gotUp && gotBreak && gotDown) {
    Serial.println("Shake!");

    // reset the byte data
    shakeByte = 0;
  }

  valx_p = valx;
  valy_p = valy;
  valz_p = valz;
  
  delay(diffTime);
}
