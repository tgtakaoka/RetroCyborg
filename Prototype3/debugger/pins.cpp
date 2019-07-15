#include <avr/pgmspace.h>
#include <Arduino.h>

#include "hex.h"
#include "pins.h"
#include "pins_map.h"

class Pins Pins;

#define BV(name)   _BV(name ## _BV)
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

#define INPUT_DB(n)  pinMode(DB##n, INPUT)
#define OUTPUT_DB(n) pinMode(DB##n, OUTPUT)
#define READ_DB(n, v) do {               \
  if (digitalRead(DB##n)) (v) |= _BV(n); \
} while (0)
#define WRITE_DB(n, v) do {                    \
  pinMode(DB##n, OUTPUT);                      \
  digitalWrite(DB##n, LOW);                    \
  if ((v) & _BV(n)) digitalWrite(DB##n, HIGH); \
} while (0)

uint8_t Pins::Dbus::getDbus() {
  uint8_t data = 0;
  READ_DB(0, data);
  READ_DB(1, data);
  READ_DB(2, data);
  READ_DB(3, data);
  READ_DB(4, data);
  READ_DB(5, data);
  READ_DB(6, data);
  READ_DB(7, data);
  return data;
}

void Pins::Dbus::begin() {
  setDbus(INPUT, 0);
}

void Pins::Dbus::setDbus(uint8_t dir, uint8_t data) {
  if (dir == OUTPUT && digitalRead(RD_WR) == LOW) {
    Serial.println(F("!! R/W is LOW"));
    return;
  }
  if (dir == OUTPUT || _capture) {
    digitalWrite(RAM_E, HIGH); // disable RAM
  } else {
    digitalWrite(RAM_E, LOW);  // enable RAM
  }
  _dir = dir;
  if (dir == INPUT) {
    INPUT_DB(0);
    INPUT_DB(1);
    INPUT_DB(2);
    INPUT_DB(3);
    INPUT_DB(4);
    INPUT_DB(5);
    INPUT_DB(6);
    INPUT_DB(7);
  } else {
    WRITE_DB(0, data);
    WRITE_DB(1, data);
    WRITE_DB(2, data);
    WRITE_DB(3, data);
    WRITE_DB(4, data);
    WRITE_DB(5, data);
    WRITE_DB(6, data);
    WRITE_DB(7, data);
  }
}

void Pins::Dbus::input() {
  setDbus(INPUT, 0);
}

void Pins::Dbus::output() {
  if (_valid) {
    _valid = false;
    setDbus(OUTPUT, _data);
  } else {
    input();
  }
}

void Pins::Dbus::set(uint8_t data) {
  _data = data;
  _valid = true;
}

void Pins::Dbus::capture(bool enabled) {
  _capture = enabled;
}

void Pins::Status::get() {
  uint8_t p = 0;
  if (digitalRead(RESET)) p |= Status::reset;
  if (digitalRead(HALT))  p |= Status::halt;
  if (digitalRead(BA))    p |= Status::ba;
  if (digitalRead(BS))    p |= Status::bs;
  if (digitalRead(LIC))   p |= Status::lic;
  if (digitalRead(AVMA))  p |= Status::avma;
  if (digitalRead(RD_WR)) p |= Status::rw;
  pins = p;
  dbus  = Dbus::getDbus();
}

bool Pins::unchanged() const {
  return _signals.pins == _previous.pins
      && _signals.dbus == _previous.dbus;
}

static void printPin(uint8_t value, const __FlashStringHelper *name) {
  Serial.print(name);
  Serial.print(value ? 'H' : 'L');
}

void Pins::print() const {
  printPin(_signals.pins & Status::reset, F(" #RES="));
  printPin(_signals.pins & Status::halt,  F(" HALT="));
  printPin(_signals.pins & Status::ba,    F(" BA="));
  printPin(_signals.pins & Status::bs,    F(" BS="));
  printPin(_signals.pins & Status::lic,   F(" LIC="));
  printPin(_signals.pins & Status::avma,  F(" AVMA="));
  printPin(_signals.pins & Status::rw,    F(" RW="));

  Serial.print(F(" DB=0x"));
  printHex2(_signals.dbus);
  Serial.print(' ');
  if (vectorFetch()) {
    Serial.print('V');
  } else if (running()) {
    if (writeCycle()) {
      Serial.print('W');
    } else if (readCycle()) {
      Serial.print('R');
    } else {
      Serial.print('-');
    }
  } else {
    Serial.print('H');
  }
  Serial.println();
}

bool Pins::inHalt() const {
  return (_signals.pins & Status::ba) && (_signals.pins & Status::bs);
}

bool Pins::vectorFetch() const {
  return (_signals.pins & (Status::ba | Status::bs)) == Status::bs;
}

bool Pins::running() const {
  return (_signals.pins & (Status::ba | Status::bs)) == 0;
}

bool Pins::lastInstCycle() const {
  return _signals.pins & Status::lic;
}

bool Pins::writeCycle() const {
  return (_previous.pins & Status::avma) && (_signals.pins & Status::rw) == 0;
}

bool Pins::readCycle() const {
  return (_previous.pins & Status::avma) && (_signals.pins & Status::rw);
}

void Pins::reset() {
  digitalWrite(RESET, LOW);
  digitalWrite(HALT, HIGH);
  delayMicroseconds(10);
  digitalWrite(STEP, LOW);
  digitalWrite(RESET, HIGH);
  do {
    cycle();
    digitalWrite(HALT, LOW);
    print();
  } while (!inHalt());
  digitalWrite(STEP, HIGH);
}

void Pins::cycle() {
  _previous = _signals;
  _signals.get();
  if (running()) {
    if (writeCycle()) {
      _dbus.input();
    } else if (readCycle()) {
      _dbus.output();
    }
  } else {
    _dbus.input();
  }
  _signals.get();
  digitalWrite(ACK, LOW);
  delayMicroseconds(4);
  _dbus.input();
  digitalWrite(ACK, HIGH);
  delayMicroseconds(4);
}

void Pins::run() {
  digitalWrite(HALT, HIGH);
  digitalWrite(STEP, HIGH);
}

void Pins::halt(bool show) {
  digitalWrite(STEP, LOW);
  delayMicroseconds(10);
  digitalWrite(HALT, LOW);
  do {
    cycle();
    if (show) print();
  } while (running());
  digitalWrite(STEP, HIGH);
}

void Pins::setData(uint8_t data) {
  _dbus.set(data);
}

void Pins::unhalt() {
  digitalWrite(STEP, LOW);
  delayMicroseconds(10);
  digitalWrite(HALT, HIGH);
  do {
    cycle();
    digitalWrite(HALT, LOW);
  } while (!running() || !lastInstCycle());
}

void Pins::execInst(const uint8_t *inst, uint8_t len, bool show) {
  unhalt();
  for (uint8_t i = 0; i < len; i++) {
    setData(inst[i]);
    cycle();
    if (show) print();
  }
  while (!lastInstCycle()) {
    cycle();
    if (show) print();
  }
  digitalWrite(STEP, HIGH);
}

void Pins::captureWrites(const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max) {
  unhalt();
  for (uint8_t i = 0; i < len; i++) {
    setData(inst[i]);
    cycle();
  }
  _dbus.capture(true);
  while (!lastInstCycle()) {
    cycle();
    if (writeCycle() && max > 0) {
      *buf++ = _signals.dbus;
      max--;
    }
  }
  _dbus.capture(false);
  digitalWrite(STEP, HIGH);
}

void Pins::step(bool show) {
  unhalt();
  do {
    cycle();
    if (show) print();
  } while (!lastInstCycle());
  digitalWrite(STEP, HIGH);
}

void Pins::begin() {
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  pinMode(HALT,  OUTPUT);
  digitalWrite(HALT, LOW);

  pinMode(STEP, OUTPUT);
  pinMode(ACK, OUTPUT);
  pinMode(INT, INPUT_PULLUP);
  digitalWrite(STEP, LOW);
  digitalWrite(ACK, HIGH);

  pinMode(BS,    INPUT);
  pinMode(BA,    INPUT);
  pinMode(LIC,   INPUT);
  pinMode(AVMA,  INPUT);
  pinMode(RD_WR, INPUT_PULLUP);
  pinMode(RAM_E, OUTPUT);
  digitalWrite(RAM_E, HIGH);

  pinMode(ADR0, INPUT_PULLUP);
  pinMode(ADR1, INPUT_PULLUP);

  _dbus.begin();

  _previous.get();

  reset();
}

void Pins::attachIoRequest(void (*isr)()) {
  attachInterrupt(INT_INTERRUPT, isr, FALLING);
}

void Pins::acknowledgeIoRequest() {
  _previous = _signals;
  _signals.get();
  digitalWrite(ACK, LOW);
}

void Pins::leaveIoRequest() {
  _dbus.input();
  digitalWrite(ACK, HIGH);
}

uint16_t Pins::ioRequestAddress() {
  uint16_t addr = 0xFFC0;
  if (digitalRead(ADR0)) addr |= 0x01;
  if (digitalRead(ADR1)) addr |= 0x02;
  return addr;
}

bool Pins::ioRequestWrite() {
  return digitalRead(RD_WR) == LOW;
}

uint8_t Pins::ioGetData() {
  _dbus.input();
  return Dbus::getDbus();
}

void Pins::ioSetData(uint8_t data) {
  _dbus.set(data);
  _dbus.output();
}
