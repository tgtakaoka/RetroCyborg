/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */

#include "console.h"

void Console::begin(HardwareSerial &stream) {
  _stream = &stream;
  reset();
}

void Console::reset() {
  _out = 0;
}

void Console::flush() {
  _stream->write(_buffer, _out);
  reset();
}

size_t Console::print(const char letter) {
  if (_out < sizeof(_buffer)) {
    _buffer[_out++] = letter;
    return 1;
  }
  return 0;
}

size_t Console::print(const char *text, size_t width) {
  size_t n = 0;
  while (*text)
    n += print(*text++);
  while (n < width--)
    n += print(' ');
  return n;
}

size_t Console::print(const __FlashStringHelper *text, size_t width) {
  PGM_P p = reinterpret_cast<PGM_P>(text);
  size_t n = 0;
  while (true) {
    const char c = static_cast<char>(pgm_read_byte(p++));
    if (c == 0) break;
    n += print(c);
  }
  while (n < width--)
    n += print(' ');
  return n;
}

size_t Console::println(const __FlashStringHelper *text) {
  size_t n = 0;
  if (text) n += print(text);
  n += print(F("\r\n"));
  flush();
  return n;
}

void Console::hex4(uint8_t val4) {
  val4 &= 0xf;
  const char c = val4 < 10 ? val4 + '0' : val4 - 10 + 'A';
  print(c);
}

void Console::hex8(uint8_t val8) {
  hex4(val8 >> 4);
  hex4(val8);
}

void Console::hex16(uint16_t val16) {
  hex8(static_cast<uint8_t>(val16 >> 8));
  hex8(static_cast<uint8_t>(val16));
}

void Console::print(uint8_t val8) {
  print(static_cast<uint32_t>(val8));
}

void Console::print(uint16_t val16) {
  print(static_cast<uint32_t>(val16));
}

void Console::print(uint32_t val32) {
  char buffer[4 * sizeof(uint32_t) + 1];
  char *p = &buffer[sizeof(buffer) - 1];
  *p = 0;
  do {
    *--p = (val32 % 10) + '0';
    val32 /= 10;
  } while (val32);
  print(p);
}

int Console::read() {
  return _stream->read();
}

int Console::available() const {
  return _stream->available();
}

size_t Console::write(uint8_t data) {
  return _stream->write(data);
}

int Console::availableForWrite() const {
  return _stream->availableForWrite();
}

class Console Console;
