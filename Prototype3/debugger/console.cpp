/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */

#include "console.h"

#include <Arduino.h>

char *outText(char *p, const char *text) {
  while (*text)
    *p++ = *text++;
  *p = 0;
  return p;
}

char toHex4(uint8_t value) {
  value &= 0x0f;
  return value < 10 ? value + '0' : value + 'A' - 10;
}

char *outHex8(char *p, uint8_t value, char suffix) {
  *p++ = toHex4(value >> 4);
  *p++ = toHex4(value);
  if (suffix) *p++ = suffix;
  *p = 0;
  return p;
}

char *outHex16(char *p, uint16_t value, char suffix) {
  p = outHex8(p, static_cast<uint8_t>(value >> 8));
  return outHex8(p, static_cast<uint8_t>(value), suffix);
}

void printHex8(uint8_t value, char suffix) {
  char buffer[4];
  outHex8(buffer, value, suffix);
  Console.print(buffer);
}

void printHex16(uint16_t value, char suffix) {
  char buffer[6];
  outHex16(buffer, value, suffix);
  Console.print(buffer);
}
