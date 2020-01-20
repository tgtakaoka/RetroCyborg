/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */

#include "console.h"

#include <Arduino.h>

void printHex8(uint8_t value, char suffix) {
  if (value < 0x10) Console.print('0');
  Console.print(value, HEX);
  if (suffix != 0) Console.print(suffix);
}

void printHex16(uint16_t value, char suffix) {
  printHex8(value >> 8);
  printHex8(value, suffix);
}
