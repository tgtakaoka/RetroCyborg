/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#include "string_util.h"

#include <Arduino.h>

char *outText(char *p, const char *text) {
  const char *t = text;
  while (true) {
    uint8_t c = *t++;
    *p = c;
    if (c == 0) return p;
    p++;
  }
}

static char toHex4(uint8_t val) {
  val &= 0xf;
  return val < 10 ? val + '0' : val - 10 + 'A';
}

char *outHex8(char *p, uint8_t val) {
  *p++ = toHex4(val >> 4);
  *p++ = toHex4(val);
  *p = 0;
  return p;
}

char *outHex16(char *p, uint16_t val) {
  p = outHex8(p, static_cast<uint8_t>(val >> 8));
  return outHex8(p, static_cast<uint8_t>(val));
}
