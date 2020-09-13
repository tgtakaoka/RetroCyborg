/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#include <Arduino.h>

#include "pins.h"
#include "pins_base.h"
#include "pins_map.h"

class Pins Pins;

void Pins::begin() {
  pinMode(CLK_E, OUTPUT);
  pinMode(CLK_Q, OUTPUT);
  pinMode(INT, OUTPUT);
  pinMode(ACK, INPUT_PULLUP);
  pinMode(STEP, INPUT_PULLUP);
#if defined(IOR_PIN)
  pinMode(IOR, INPUT);
#endif
#if defined(IO_ADRH)
  busMode(ADRH, INPUT_PULLUP);
#endif
#if defined(IO_ADRM)
  busMode(ADRM, INPUT_PULLUP);
#endif
#if defined(IO_ADRL)
  busMode(ADRL, INPUT_PULLUP);
#endif
}

bool Pins::isIoAddr() const {
#if defined(IOR_PIN)
  return digitalRead(IOR) == LOW;
#else
  return busRead(ADRH) == IO_ADRH
#if defined(IO_ADRM)
    && busRead(ADRM) == IO_ADRM
#endif
#if defined(IO_ADRL)
    && busRead(ADRL) == IO_ADRL
#endif
    ;
#endif
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
