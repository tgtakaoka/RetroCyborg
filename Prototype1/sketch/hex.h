#ifndef __HEX_H__
#define __HEX_H__

static inline void printHex2(uint8_t value) {
  if (value < 0x10) Serial.print('0');
  Serial.print(value, HEX);
}

static inline void printHex4(uint16_t value) {
  if (value < 0x1000) Serial.print('0');
  if (value < 0x100) Serial.print('0');
  if (value < 0x10) Serial.print('0');
  Serial.print(value, HEX);
}

static inline uint16_t toUint16(uint8_t *bytes) {
  uint16_t value = bytes[0];
  value <<= 8;
  return value + bytes[1];
}

#endif
