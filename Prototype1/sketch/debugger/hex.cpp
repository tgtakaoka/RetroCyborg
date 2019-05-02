#include <Arduino.h>

#include "hex.h"

void printHex2(uint8_t value, char suffix) {
  if (value < 0x10) Serial.print('0');
  Serial.print(value, HEX);
  if (suffix != 0) Serial.print(suffix);
}

void printHex4(uint16_t value, char suffix) {
  printHex2(value >> 8);
  printHex2(value, suffix);
}
