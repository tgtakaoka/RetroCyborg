#include <Arduino.h>

#include "clock.h"
#include "pins.h"
#include "pins_map.h"

class Clock Clock;

#define MSG_STOP F("Clock stop")
#define MSG_RUN  F("Clock running")

static inline void clockE(const uint8_t value) {
  digitalWrite(CLK_E, value);
}

static inline void clockQ(const uint8_t value) {
  digitalWrite(CLK_Q, value);
}

bool Clock::isRunning() {
  return _running;
}

void Clock::stop() {
  if (_running) Serial.println(MSG_STOP);
  _running = false;
}

void Clock::run() {
  if (!_running) Serial.println(MSG_RUN);
  _running = true;
}

void Clock::cycle(uint16_t ms) {
  delay(ms);
  clockE(LOW);
  Pins.setDataBusDirection(INPUT_PULLUP);
  delayMicroseconds(1);
  clockQ(HIGH);
  delayMicroseconds(1);
  clockE(HIGH);
  delayMicroseconds(1);
  clockQ(LOW);
}
