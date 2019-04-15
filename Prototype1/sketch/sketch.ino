/**
   Cyborg09 controller

   This sketch is designed for SparkFun Pro Micro 16MHz 5V and controll
   vanilla HD63C09E MPU.

   USB Serial port is used as console. So one can connect with
     $ screen /dev/<tty USB> 9600

   The Controller can accept commands represented by one letter.

   r - assert #RES to LOW.
   R - negate #RES to HIGH.
   h - assert #HALT to LOW.
   H - negate #HALT to HIGH.
   k - stop negerating clock E and Q.
   K - start free running clock E and Q.
   c - one bus cycle. stopped at E=HIGH Q=LOW.
   s - print 6309 hardware signal status.
   S - print 6309 registers.
   d - set data bus, input 2 digits hex number.
   a - set A register, input 2 digits hex number.
   x - set X register, input 4 digits hex number.
   ? - print version.
*/

#include "clock.h"
#include "input.h"
#include "pins.h"

const char * const VERSION = "* Cyborg09 Prototype1 0.3";

void setup() {
  Serial.begin(9600);
  setupPins();
}

static void handleSetDataBus(uint16_t value) {
  if (setDataBusDirection(OUTPUT)) {
    setDataBus((uint8_t) value);
  }
  printStatus();
}

enum InputMode processCharCommand() {
  char c = Serial.read();
  if (c == 's') printStatus();
  if (c == 'r') reset(LOW);
  if (c == 'R') reset(HIGH);
  if (c == 'h') halt(LOW);
  if (c == 'H') halt(HIGH);
  if (c == 'k') clock(false, "Clock stop");
  if (c == 'K') clock(true,  "Clock running");
  if (c == 'd') return readHexNumber(c, handleSetDataBus);
  if (c == 'c') {
    clockCycle(1);
    printStatus();
  } else if (isClockRunning()) {
    clockCycle(1000);
    printStatus();
  }
  if (c == '?') Serial.println(VERSION);
  return CHAR_COMMAND;
}

static enum InputMode mode = CHAR_COMMAND;

void loop() {
  switch (mode) {
  case CHAR_COMMAND:
    mode = processCharCommand();
    break;
  case HEX_NUMBER:
    mode = processHexNumber();
    break;
  }
}
