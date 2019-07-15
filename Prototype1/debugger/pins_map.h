#ifndef __pins_map_h__
#define __pins_map_h__

#include <stdint.h>

#if defined(ARDUINO_AVR_PROMICRO)
/* SparkFun Pro Micro 5V 16MHz */
const uint8_t HALT  = 1;  // TXO/D1
const uint8_t BS    = 0;  // RXI/D0
const uint8_t BA    = 2;  // SDA/D2
const uint8_t LIC   = 3;  // SCL/D3
const uint8_t AVMA  = 4;  // D4
const uint8_t RESET = 6;  // D6
const uint8_t CLK_Q = 7;  // D7
const uint8_t CLK_E = 8;  // D8
const uint8_t RD_WR = 9;  // D9
const uint8_t RAM_E = 5;  // D5

const uint8_t DBUS[] PROGMEM = {
  21, // bit 0, A3
  20, // bit 1, A2
  19, // bit 2, A1
  18, // bit 3, A0
  15, // bit 4, SCK/D15
  14, // bit 5, MISO/D14
  16, // bit 6, MOSI/D16
  10, // bit 7, D10
};
#endif

#if defined(ARDUINO_AVR_ITSYBITSY32U4_5V)
/* Adafruit ItsyBitsy 32u4 5V */
const uint8_t HALT  = 18; // A0
const uint8_t BS    = 19; // A1
const uint8_t BA    = 20; // A2
const uint8_t LIC   = 21; // A3
const uint8_t AVMA  = 22; // A4
const uint8_t RESET = 15; // SCK
const uint8_t CLK_Q = 16; // MOSI
const uint8_t CLK_E = 14; // MISO
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
