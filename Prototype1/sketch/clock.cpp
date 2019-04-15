#include <Arduino.h>

#include "clock.h"
#include "pins.h"
#include "pins_map.h"

static bool clockRunning = false;

bool isClockRunning() {
  return clockRunning;
}

static void clockE(const uint8_t value) {
  digitalWrite(CLK_E, value);
}

static void clockQ(const uint8_t value) {
  digitalWrite(CLK_Q, value);
}

void clock(const bool enable, const char *const message) {
  clockRunning = enable;
  Serial.println(message);
}

void clockCycle(int16_t ms) {
  delay(ms);
  clockE(LOW);
  setDataBusDirection(INPUT_PULLUP);
  delay(1);
  clockQ(HIGH);
  delay(1);
  clockE(HIGH);
  delay(1);
  clockQ(LOW);
}
