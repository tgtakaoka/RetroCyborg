#include <avr/pgmspace.h>
#include <Arduino.h>

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

static uint8_t pinDataBus(uint8_t bit) {
  return pgm_read_byte_near(DBUS + bit);
}

static uint8_t getDbus() {
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

static void setDbus(uint8_t dir, uint8_t data) {
  if (dir == OUTPUT && digitalRead(RD_WR) == LOW) {
    Serial.println("!! R/W is LOW");
    return;
  }
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

bool Pins::Status::equals(const Status& o) const {
  return _reset == o._reset
         && _halt ==  o._halt
         && _ba ==    o._ba
         && _bs ==    o._bs
         && _lic ==   o._lic
         && _avma ==  o._avma
         && _busy ==  o._busy
         && _rw ==    o._rw
         && _dbus ==  o._dbus;
}

void Pins::Status::get() {
  _cycle = Pins::_cycle;
  _reset = digitalRead(RESET);
  _halt  = digitalRead(HALT);
  _ba    = digitalRead(BA);
  _bs    = digitalRead(BS);
  _lic   = digitalRead(LIC);
  _avma  = digitalRead(AVMA);
  _busy  = digitalRead(BUSY);
  _rw    = digitalRead(RD_WR);
  _dbus  = getDbus();
}

static void printPin(uint8_t value, const __FlashStringHelper *name) {
  Serial.print(name);
  Serial.print(value ? 'H' : 'L');
}

static void printHex2(uint8_t value) {
  if (value < 0x10) Serial.print('0');
  Serial.print(value, HEX);
}

void Pins::Status::print(bool nl) const {
  Serial.print(_cycle);
  printPin(_reset, MSG_RES);
  printPin(_halt,  MSG_HALT);
  printPin(_ba,    MSG_BA);
  printPin(_bs,    MSG_BS);
  printPin(_lic,   MSG_LIC);
  printPin(_avma,  MSG_AVMA);
  printPin(_busy,  MSG_BUSY);
  printPin(_rw,    MSG_RW);
  Serial.print(MSG_DBUS);
  printHex2(_dbus);
  if (busWrite()) {
    Serial.print(' ');
    Serial.print('W');
  }
  if (vectorFetch()) {
    Serial.print(' ');
    Serial.print('V');
    Serial.print(busBusy() ? 'H' : 'L');
  }
  if (nl) Serial.println();
  else Serial.print(' ');
}

void Pins::reset(uint8_t value) {
  digitalWrite(RESET, value);
}

void Pins::halt(uint8_t value) {
  digitalWrite(HALT, value);
}

void Pins::setVector(uint8_t vh, uint8_t vl) {
  _vector_high = vh;
  _vector_low = vl;
}

void Pins::Dbus::input() {
  if (_dir == INPUT) return;
  setDbus(INPUT, 0);
  _dir = INPUT;
}

void Pins::Dbus::output() {
  if (!_data_valid) return;
  setDbus(OUTPUT, _data);
  _data_valid = false;
  _dir = OUTPUT;
}

void Pins::Dbus::set(uint8_t data) {
  _data = data;
  _data_valid = true;
}

uint16_t Pins::_cycle;

void Pins::cycle(Status& pins) {
  _cycle++;
  digitalWrite(CLK_Q, HIGH);
  digitalWrite(CLK_E, HIGH);
  digitalWrite(CLK_Q, LOW);
  pins.get();
  if (pins.running()) {
    if (pins.busWrite()) {
      _dbus.input();
    } else {
      _dbus.output();
    }
  } else if (pins.vectorFetch()) {
    if (pins.busBusy()) {
      _dbus.set(_vector_high);
      _dbus.output();
    } else {
      _dbus.set(_vector_low);
      _dbus.output();
    }
  } else {
    _dbus.input();
  }
  pins.get();
  digitalWrite(CLK_E, LOW);
  _dbus.input();
}

void Pins::print(bool nl) const {
  Status pins;
  pins.get();
  pins.print(nl);
}

void Pins::begin(Status& pins) {
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

  setDbus(INPUT, 0);

  pins.get();
}
