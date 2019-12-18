/**
   Cyborg09 controller

   This sketch is designed for Adafruit ItsyBitsy 32u4 16MHz 5V and
   controll vanilla HD63C09E MPU.

   USB Serial port is used as console. So one can connect with
     $ screen /dev/<tty USB> 9600
*/

#include "commands.h"
#include "mc6850.h"
#include "input.h"
#include "pins.h"

class Mc6850 Mc6850(Pins::ioBaseAddress());

void setup() {
  Serial.begin(115200);
  Pins.begin();
  Pins.attachIoRequest(ioRequest);
  Input.begin();
  interrupts();
}

void loop() {
  if (Commands.isRunning()) {
    Mc6850.loop();
  } else {
    Input.loop();
  }
}

static uint8_t data;

void ioRequest() {
  const uint16_t ioAddr = Pins.ioRequestAddress();
  const bool ioWrite = Pins.ioRequestWrite();
  if (Mc6850.isSelected(ioAddr)) {
    if (ioWrite) Mc6850.write(Pins.ioGetData(), ioAddr);
    else Pins.ioSetData(Mc6850.read(ioAddr));
  } else {
    if (ioWrite) data = Pins.ioGetData();
    else Pins.ioSetData(data);
  }

  Pins.acknowledgeIoRequest();
  Pins.leaveIoRequest();
}
