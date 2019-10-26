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

#define __concat2__(a,b) a##b
#define BP(name)  name##_PIN
#define BM(name)   _BV(BP(name))
#define BUS(name)  (name ## _BUS)
#define __PORT__(name) name##_PORT
#define __DDR__(port) __concat2__(DDR,port)
#define __POUT__(port) __concat2__(PORT,port)
#define __PIN__(port) __concat2__(PIN,port)
#define DDR(name) __DDR__(__PORT__(name))
#define PORT(name) __POUT__(__PORT__(name))
#define PIN(name) __PIN__(__PORT__(name))
#define pinMode(name, mode) do {                    \
  if (mode == INPUT) DDR(name) &= ~BM(name);        \
  if (mode == INPUT_PULLUP) DDR(name) &= ~BM(name); \
  if (mode == INPUT_PULLUP) PORT(name) |= BM(name); \
  if (mode == OUTPUT) DDR(name) |= BM(name);        \
} while (0)
#define digitalRead(name) (PIN(name) & BM(name))
#define digitalWrite(name, val) do {       \
  if (val == LOW) PORT(name) &= ~BM(name); \
  if (val == HIGH) PORT(name) |= BM(name); \
} while (0)
#define busMode(name, mode) do {                     \
  if (mode == INPUT) DDR(name) &= ~BUS(name);        \
  if (mode == INPUT_PULLUP) DDR(name) &= ~BUS(name); \
  if (mode == INPUT_PULLUP) PORT(name) |= BUS(name); \
  if (mode == OUTPUT) DDR(name) |= BUS(name);         \
} while (0)
#define busRead(name) (PIN(name) & BUS(name))
#define busWrite(name, val) \
    (PORT(name) = (PORT(name) & ~(BUS(name)) | (val & BUS(name))))

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
