#include <avr/pgmspace.h>
#include <Arduino.h>

#define DBUS_PULL_DOWN 1

#include "pins.h"
#include "pins_map.h"

class Pins Pins;

static void setDigital(uint8_t pin, uint8_t value, const __FlashStringHelper *name) {
  digitalWrite(pin, value);
  Serial.print(name);
  Serial.println(value ? 'H' : 'L');
}

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

void Pins::reset(uint8_t value) {
  setDigital(RESET, value, MSG_RES);
}

void Pins::halt(uint8_t value) {
  setDigital(HALT, value, MSG_HALT);
}


static uint8_t pinDataBus(uint8_t bit) {
  return pgm_read_byte_near(DBUS + bit);
}

bool Pins::setDataBusDirection(uint8_t dir) {
  if (dir == OUTPUT && digitalRead(RD_WR) == LOW) {
    Serial.println(MSG_DBUS_ERR);
    return false;
  }
#if !defined(DBUS_PULL_DOWN)
  for (uint8_t bit = 0; bit < 8; bit++) {
    pinMode(pinDataBus(bit), dir);
  }
#endif
  return true;
}

void Pins::setDataBus(uint8_t data) {
  for (uint8_t bit = 0; bit < 8; bit++) {
    const uint8_t pin = pinDataBus(bit);
#if defined(DBUS_PULL_DOWN)
    if (data & 0x01) {
      pinMode(pin, INPUT_PULLUP);
      digitalWrite(pin, HIGH);
    } else {
      pinMode(pin, INPUT);
    }
#else
    if (data & 0x01) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
#endif
    data >>= 1;
  }
}

uint8_t Pins::getDataBus() {
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

static void printPinStatus(uint8_t pin, const __FlashStringHelper *name) {
  Serial.print(name);
  Serial.print(digitalRead(pin) ? 'H' : 'L');
}

static void Pins::printStatus() {
  printPinStatus(RESET, MSG_RES);
  printPinStatus(HALT,  MSG_HALT);
  printPinStatus(BA,    MSG_BA);
  printPinStatus(BS,    MSG_BS);
  printPinStatus(LIC,   MSG_LIC);
  printPinStatus(AVMA,  MSG_AVMA);
  printPinStatus(BUSY,  MSG_BUSY);
  printPinStatus(RD_WR, MSG_RW);
  Serial.print(MSG_DBUS);
  Serial.println(getDataBus(), HEX);
}

void Pins::begin() {
  pinMode(RESET, OUTPUT);
  reset(LOW);
  pinMode(HALT,  OUTPUT);
  halt(LOW);

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

  setDataBusDirection(INPUT);
}
