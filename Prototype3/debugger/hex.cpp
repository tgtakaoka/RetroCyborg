#include <Arduino.h>

#include "hex.h"

void printHex8(uint8_t value, char suffix) {
  if (value < 0x10) Serial.print('0');
  Serial.print(value, HEX);
  if (suffix != 0) Serial.print(suffix);
}

void printHex16(uint16_t value, char suffix) {
  printHex8(value >> 8);
  printHex8(value, suffix);
}
