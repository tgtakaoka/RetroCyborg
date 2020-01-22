/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#include <Arduino.h>

#include "pins.h"
#include "pins_map.h"

class Pins Pins;

#define PIN_m(name)   _BV(__PIN__(name))
#define pinMode(name, mode) do {                          \
    if (mode == INPUT) DDR(name) &= ~PIN_m(name);         \
    if (mode == INPUT_PULLUP) DDR(name) &= ~PIN_m(name);  \
    if (mode == INPUT_PULLUP) POUT(name) |= PIN_m(name);  \
    if (mode == OUTPUT) DDR(name) |= PIN_m(name);         \
  } while (0)
#define digitalRead(name) (PIN(name) & PIN_m(name))
#define digitalWrite(name, val) do {            \
    if (val == LOW) POUT(name) &= ~PIN_m(name); \
    if (val == HIGH) POUT(name) |= PIN_m(name); \
  } while (0)
#define busMode(name, mode) do {                            \
    if (mode == INPUT) DDR(name) &= ~__BUS__(name);         \
    if (mode == INPUT_PULLUP) DDR(name) &= ~__BUS__(name);  \
    if (mode == INPUT_PULLUP) POUT(name) |= __BUS__(name);  \
    if (mode == OUTPUT) DDR(name) |= __BUS__(name);         \
  } while (0)
#define busRead(name) (PIN(name) & __BUS__(name))
#define busWrite(name, val)                                             \
  (POUT(name) = (POUT(name) & ~(__BUS__(name)) | (val & __BUS__(name))))

void Pins::begin() {
  pinMode(CLK_E, OUTPUT);
  pinMode(CLK_Q, OUTPUT);
  digitalWrite(CLK_E, LOW);
  digitalWrite(CLK_Q, LOW);
  pinMode(INT, OUTPUT);
  digitalWrite(INT, HIGH);
  pinMode(ACK, INPUT_PULLUP);
  pinMode(STEP, INPUT_PULLUP);
  busMode(ADRH, INPUT_PULLUP);
#if defined(IO_ADRM)
  busMode(ADRM, INPUT_PULLUP);
#endif
#if defined(IO_ADRL)
  busMode(ADRL, INPUT_PULLUP);
#endif
}

bool Pins::isIoAddr() const {
  return busRead(ADRH) == IO_ADRH
#if defined(IO_ADRM)
    && busRead(ADRM) == IO_ADRM
#endif
#if defined(IO_ADRL)
    && busRead(ADRL) == IO_ADRL
#endif
    ;
}

bool Pins::isAck() const {
  return digitalRead(ACK) == LOW;
}

bool Pins::isStep() const {
  return digitalRead(STEP) == LOW;
}

void Pins::setE() {
  digitalWrite(CLK_E, HIGH);
}

void Pins::clrE() {
  digitalWrite(CLK_E, LOW);
}

void Pins::setQ() {
  digitalWrite(CLK_Q, HIGH);
}

void Pins::clrQ() {
  digitalWrite(CLK_Q, LOW);
}

void Pins::nop() const {
#if F_CPU == 20000000
  __asm__ __volatile__ ("nop");
  __asm__ __volatile__ ("nop");
  __asm__ __volatile__ ("nop");
#endif
#if F_CPU == 16000000
  __asm__ __volatile__ ("nop");
  __asm__ __volatile__ ("nop");
#endif
}

void Pins::assertInt() {
  digitalWrite(INT, LOW);
}

void Pins::negateInt() {
  digitalWrite(INT, HIGH);
}
