/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#include <Arduino.h>

#include "pins.h"
#include "pins_map.h"

class Pins Pins;

#define pinMode(name, mode) do {                \
    if ((mode) == INPUT) {                      \
      PORT(name).DIRCLR = PIN_m(name);          \
      PINCTRL(name) &= ~PORT_PULLUPEN_bm;       \
    } else if ((mode) == INPUT_PULLUP) {        \
      PORT(name).DIRCLR = PIN_m(name);          \
      PINCTRL(name) |= PORT_PULLUPEN_bm;        \
    } else if ((mode) == OUTPUT) {              \
      PORT(name).DIRSET = PIN_m(name);          \
    }                                           \
  } while (0)
#define pinModeInvert(name) PINCTRL(name) |= PORT_INVEN_bm
#define digitalRead(name) (PIN(name) & PIN_m(name))
#define digitalWrite(name, val) do {            \
    if ((val) == LOW) {                         \
      POUT(name) &= ~PIN_m(name);               \
    } else {                                    \
      POUT(name) |= PIN_m(name);                \
    }                                           \
  } while (0)

#define BUS_gm(name) name##_BUS
static void enablePullup(register8_t *pinctrl, uint8_t mask) {
  while (mask) {
    if (mask & 1) *pinctrl |= PORT_PULLUPEN_bm;
    pinctrl++;
    mask >>= 1;
  }
}
#define busMode(name, mode) do {                        \
    if ((mode) == INPUT) {                              \
      PORT(name).DIRCLR = BUS_gm(name);                 \
    } else if ((mode) == INPUT_PULLUP) {                \
      PORT(name).DIRCLR = BUS_gm(name);                 \
      enablePullup(&PORT(name).PIN0CTRL, BUS_gm(name)); \
    } else if ((mode) == OUTPUT) {                      \
      PORT(name).DIRSET = BUS_gm(name);                 \
    }                                                   \
  } while (0)
#define busRead(name) (PIN(name) & BUS_gm(name))
#define busWrite(name, val) do {                  \
    if (BUS_gm(name) == 0xFF) {                   \
      PORT(name).OUT = (val);                     \
    } else {                                      \
      PORT(name).OUTSET =  (val) & BUS_gm(name);  \
      PORT(name).OUTCLR = ~(val) & BUS_gm(name);  \
    }                                             \
  } while (0);

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
