#include <avr/pgmspace.h>
#include <Arduino.h>

#include "hex.h"
#include "pins.h"
#include "pins_map.h"

class Pins Pins;

#define MSG_RES    F(" #RES=")
#define MSG_HALT   F(" #HALT=")
#define MSG_BA     F(" BA=")
#define MSG_BS     F(" BS=")
#define MSG_LIC    F(" LIC=")
#define MSG_AVMA   F(" AVMA=")
#define MSG_BUSY   F(" BUSY=")
#define MSG_RW     F(" RW=")
#define MSG_DBUS   F(" Dn=0x")
#define MSG_DBUS_ERR F(" !!! RW=L")
#define MSG_VECTOR F(" V")
#define MSG_WRITE  F(" W")
#define MSG_READ   F(" R")
#define MSG_INST   F(" I")

static uint8_t pinDataBus(uint8_t bit) {
  return pgm_read_byte_near(DBUS + bit);
}

uint8_t Pins::Dbus::getDbus() {
  uint8_t data = 0;
  for (uint8_t bit = 0; bit < 8; bit++) {
    data >>= 1;
    const uint8_t pin = pinDataBus(bit);
    if (digitalRead(pin) == HIGH) {
      data |= 0x80;
    }
  }
  return data;
}

void Pins::Dbus::begin() {
  setDbus(INPUT, 0);
}

void Pins::Dbus::setDbus(uint8_t dir, uint8_t data) {
  if (dir == OUTPUT && digitalRead(RD_WR) == LOW) {
    Serial.println("!! R/W is LOW");
    return;
  }
  if (dir == OUTPUT || _capture) {
    digitalWrite(RAM_E, HIGH);
  } else {
    digitalWrite(RAM_E, LOW);
  }
  _dir = dir;
  for (uint8_t bit = 0; bit < 8; bit++) {
    uint8_t pin = pinDataBus(bit);
    pinMode(pin, dir);
    if (dir == OUTPUT) {
      if (data & 1) {
        digitalWrite(pin, HIGH);
      } else {
        digitalWrite(pin, LOW);
      }
    }
    data >>= 1;
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
  reset = digitalRead(RESET);
  halt  = digitalRead(HALT);
  ba    = digitalRead(BA);
  bs    = digitalRead(BS);
  lic   = digitalRead(LIC);
  avma  = digitalRead(AVMA);
  busy  = digitalRead(BUSY);
  rw    = digitalRead(RD_WR);
  dbus  = Dbus::getDbus();
}

bool Pins::unchanged() const {
  return _signals.reset == _previous.reset
         && _signals.halt ==  _previous.halt
         && _signals.ba ==    _previous.ba
         && _signals.bs ==    _previous.bs
         && _signals.lic ==   _previous.lic
         && _signals.avma ==  _previous.avma
         && _signals.busy ==  _previous.busy
         && _signals.rw ==    _previous.rw
         && _signals.dbus ==  _previous.dbus;
}

static void printPin(uint8_t value, const __FlashStringHelper *name) {
  Serial.print(name);
  Serial.print(value ? 'H' : 'L');
}

void Pins::print() const {
  Serial.print(_cycle);
  printPin(_signals.reset, MSG_RES);
  printPin(_signals.halt,  MSG_HALT);
  printPin(_signals.ba,    MSG_BA);
  printPin(_signals.bs,    MSG_BS);
  printPin(_signals.lic,   MSG_LIC);
  printPin(_signals.avma,  MSG_AVMA);
  printPin(_signals.busy,  MSG_BUSY);
  printPin(_signals.rw,    MSG_RW);
  Serial.print(MSG_DBUS);
  printHex2(_signals.dbus);
  if (vectorFetch()) {
    Serial.print(MSG_VECTOR);
  } else if (running()) {
    if (writeCycle()) {
      Serial.print(MSG_WRITE);
    } else if (readCycle()) {
      if (_signals.inst) {
        Serial.print(MSG_INST);
      } else {
        Serial.print(MSG_READ);
      }
    }
  }
  Serial.println();
}

bool Pins::inHalt() const {
  return _signals.ba && _signals.bs;
}

bool Pins::vectorFetch() const {
  return !_signals.ba && _signals.bs;
}

bool Pins::running() const {
  return !_signals.ba && !_signals.bs;
}

bool Pins::lastInstCycle() const {
  return _signals.lic && _signals.avma;
}

bool Pins::writeCycle() const {
  return _previous.avma && !_signals.rw;
}

bool Pins::readCycle() const {
  return _previous.avma && _signals.rw;
}

void Pins::reset() {
  digitalWrite(RESET, LOW);
  cycle();
  print();
  digitalWrite(RESET, HIGH);
  cycle();
  print();
  digitalWrite(HALT, HIGH);
  cycle();
  print();
  digitalWrite(HALT, LOW);
  do {
    cycle();
    print();
  } while (!inHalt());
}

void Pins::setData(uint8_t data) {
  _dbus.set(data);
}

void Pins::cycle() {
  _cycle++;
  _previous = _signals;
  digitalWrite(CLK_Q, HIGH);
  digitalWrite(CLK_E, HIGH);
  digitalWrite(CLK_Q, LOW);
  _signals.get();
  if (running()) {
    if (writeCycle()) {
      _dbus.input();
    } else if (readCycle()) {
      _signals.inst = _dbus.valid();
      _dbus.output();
    }
  } else {
    _dbus.input();
  }
  _signals.get();
  digitalWrite(CLK_E, LOW);
  _dbus.input();
}

void Pins::unhalt() {
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
}

void Pins::captureWrites(const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max) {
  unhalt();
  for (uint8_t i = 0; i < len; i++) {
    setData(inst[i]);
    cycle();
//    print();
  }
  _dbus.capture(true);
  while (!lastInstCycle()) {
    cycle();
    if (writeCycle() && max > 0) {
      *buf++ = _signals.dbus;
      max--;
    }
//    print();
  }
  _dbus.capture(false);
}

void Pins::step(bool show) {
  unhalt();
  do {
    cycle();
    if (show) print();
  } while (!lastInstCycle());
}

void Pins::begin() {
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  pinMode(HALT,  OUTPUT);
  digitalWrite(HALT, LOW);

  pinMode(CLK_E, OUTPUT);
  pinMode(CLK_Q, OUTPUT);
  digitalWrite(CLK_E, LOW);
  digitalWrite(CLK_Q, LOW);

  pinMode(BS,    INPUT);
  pinMode(BA,    INPUT);
  pinMode(LIC,   INPUT);
  pinMode(AVMA,  INPUT);
  pinMode(BUSY,  INPUT);
  pinMode(RD_WR, INPUT_PULLUP);
  pinMode(RAM_E, OUTPUT);
  digitalWrite(RAM_E, HIGH);

  _dbus.begin();

  _previous.get();
}
