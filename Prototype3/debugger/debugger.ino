/**
   Cyborg09 controller

   This sketch is designed for Adafruit ItsyBitsy 32u4 16MHz 5V and
   controll vanilla HD63C09E MPU.

   USB Serial port is used as console. So one can connect with
     $ screen /dev/<tty USB> 9600
*/
#include <SD.h>

#include "input.h"
#include "pins.h"

void setup() {
  Serial.begin(115200);
  Pins.begin();
  Pins.attachIoRequest(ioRequest);
  Input.begin();
  interrupts();
  SD.begin();
}

void loop() {
  Input.loop();
}

static uint8_t data;

void ioRequest() {
  const uint16_t ioAddr = Pins.ioRequestAddress();
  const bool ioWrite = Pins.ioRequestWrite();
  if (ioWrite) data = Pins.ioGetData();
  else Pins.ioSetData(data);

  Pins.acknowledgeIoRequest();

  Serial.print(F("IO Request: "));
  if (ioWrite) {
    Serial.print(F("write="));
  } else {
    Serial.print(F(" read="));
  }
  Serial.print(ioAddr, HEX);
  Serial.print(F(" data="));
  Serial.println(data, HEX);

  Pins.leaveIoRequest();
}
