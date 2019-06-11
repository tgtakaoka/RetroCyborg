#include <Arduino.h>

// 0xFFC0 ~ 0xFFDF
#define IO_ADR_BASE 0xFFC0

#include "pins.h"
#include "pins_map.h"

class Pins Pins;

#if F_CPU == 20000000
#define nop() do { \
  __asm__ __volatile__ ("nop"); \
  __asm__ __volatile__ ("nop"); \
  __asm__ __volatile__ ("nop"); \
} while (0)
#endif
#if F_CPU == 16000000
#define nop() do { \
  __asm__ __volatile__ ("nop"); \
  __asm__ __volatile__ ("nop"); \
} while (0)
#endif

#define BV(name)   _BV(name ## _BV)
#define BUS(name)  (name ## _BUS)
#define DDR(name)  (name ## _DDR)
#define PORT(name) (name ## _PORT)
#define PIN(name)  (name ## _PIN)
#define pinMode(name, mode) do {                    \
  if (mode == INPUT) DDR(name) &= ~BV(name);        \
  if (mode == INPUT_PULLUP) DDR(name) &= ~BV(name); \
  if (mode == INPUT_PULLUP) PORT(name) |= BV(name); \
  if (mode == OUTPUT) DDR(name) |= BV(name);        \
} while (0)
#define digitalRead(name) (PIN(name) & BV(name))
#define digitalWrite(name, val) do {       \
  if (val == LOW) PORT(name) &= ~BV(name); \
  if (val == HIGH) PORT(name) |= BV(name); \
} while (0)
#define busMode(name, mode) do {                     \
  if (mode == INPUT) DDR(name) &= ~BUS(name);        \
  if (mode == INPUT_PULLUP) DDR(name) &= ~BUS(name); \
  if (mode == INPUT_PULLUP) PORT(name) |= BUS(name); \
  if (mode == OUTPUT) DDR(name) |= BUS(name);         \
} while (0)
#define busRead(name) (PIN(name) & BUS(name))
#define busWrite(name, val) \
    (POUT(name) = (POUT(name) & ~(BUS(name)) | (val & BUS(name))))

void Pins::begin() {
  pinMode(CLK_E, OUTPUT);
  pinMode(CLK_Q, OUTPUT);
  pinMode(INT, OUTPUT);
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
  nop();
}

void Pins::clrE() {
  digitalWrite(CLK_E, LOW);
}

void Pins::setQ() {
  digitalWrite(CLK_Q, HIGH);
  nop();
}

void Pins::clrQ() {
  digitalWrite(CLK_Q, LOW);
}

void Pins::assertInt() {
  digitalWrite(INT, LOW);
}

void Pins::negateInt() {
  digitalWrite(INT, HIGH);
}
