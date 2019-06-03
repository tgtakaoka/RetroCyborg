#ifndef __pins_map_h__
#define __pins_map_h__

#include <stdint.h>

#if defined(ARDUINO_AVR_ITSYBITSY32U4_5V)
/* Adafruit ItsyBitsy 32u4 5V */
const uint8_t HALT  = 18; // A0
const uint8_t BS    = 19; // A1
const uint8_t BA    = 20; // A2
const uint8_t LIC   = 21; // A3
const uint8_t AVMA  = 22; // A4
const uint8_t BUSY  = 23; // A5
const uint8_t RESET = 15; // SCK
const uint8_t STEP  = 16; // MOSI
const uint8_t ACK   = 14; // MISO
const uint8_t RD_WR = 8;  // D8
const uint8_t RAM_E = 11; // D11

const uint8_t DBUS[] PROGMEM = {
  10, // bit 0, D10
  9,  // bit 1, D9
  7,  // bit 2, D7
  5,  // bit 3, D5
  3,  // bit 4, SCL
  2,  // bit 5, SDA
  1,  // bit 6, TX
  0,  // bit 7, RX
};

#endif

#endif
