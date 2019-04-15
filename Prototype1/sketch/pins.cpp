#include <avr/pgmspace.h>
#include <Arduino.h>

#define DBUS_PULL_DOWN 1

#include "pins.h"
#include "pins_map.h"

static void setDigital(const uint8_t pin, const uint8_t value, const char *name) {
  digitalWrite(pin, value);
  Serial.print(name);
  Serial.println(value ? "HIGH" : "LOW");
}

void reset(const uint8_t value) {
  setDigital(RESET, value, "#RES=");
}

void halt(const uint8_t value) {
  setDigital(HALT, value, "#HALT=");
}

static uint8_t pinDataBus(const uint8_t bit) {
  return pgm_read_byte_near(DBUS + bit);
}

bool setDataBusDirection(const uint8_t mode) {
  if (mode == OUTPUT && digitalRead(RD_WR) == LOW) {
    Serial.println(" !! RW=LOW");
    return false;
  }
#if !defined(DBUS_PULL_DOWN)
  for (uint8_t bit = 0; bit < 8; bit++) {
    pinMode(pinDataBus(bit), mode);
  }
#endif
  return true;
}

void setDataBus(uint8_t data) {
  for (uint8_t bit = 0; bit < 8; bit++) {
    const uint8_t pin = pinDataBus(bit);
#if defined(DBUS_PULL_DOWN)
    if (data & 0x01) {
      pinMode(pin, INPUT_PULLUP);
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

uint8_t getDataBus() {
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

static void printPinStatus(const uint8_t pin, const char *title) {
  Serial.print(title);
  Serial.print(digitalRead(pin) ? "H" : "L");
}

void printStatus() {
  printPinStatus(RESET, "#RES=");
  printPinStatus(HALT,  " HALT=");
  printPinStatus(BA,    " BA=");
  printPinStatus(BS,    " BS=");
  printPinStatus(LIC,   " LIC=");
  printPinStatus(AVMA,  " AVMA=");
  printPinStatus(BUSY,  " BUSY=");
  printPinStatus(RD_WR, " RW=");
  Serial.print(" Dn=0x");
  Serial.println(getDataBus(), HEX);
}

void setupPins() {
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
