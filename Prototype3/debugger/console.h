/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __console_h__
#define __console_h__

#include <Arduino.h>
#include <stdint.h>

class Console {
public:
  Console() : _out(0) {}

  void begin(HardwareSerial &stream);
  size_t print(const char letter);
  size_t print(const char *text, size_t width = 0);
  size_t print(const __FlashStringHelper *text, size_t width = 0);
  size_t println(const __FlashStringHelper *text = nullptr);
  void print(uint8_t value);
  void print(uint16_t value);
  void print(uint32_t value);
  void hex4(uint8_t val4);
  void hex8(uint8_t val8);
  void hex16(uint16_t val16);
  void flush();

  int read();
  int available() const;
  size_t write(uint8_t data);
  int availableForWrite() const;

private:
  size_t _out;
  uint8_t _buffer[80];
  HardwareSerial *_stream;

  void reset();
  void printNumber(uint16_t val, uint8_t base);
};

extern Console Console;

#endif
