/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#include <avr/pgmspace.h>
#include <Arduino.h>

#include "console.h"
#include "mc6850.h"
#include "pins.h"
#include "pins_map.h"

class Pins Pins;
class Mc6850 Mc6850(Pins::ioBaseAddress());

static uint8_t data;

void ioRequest() {
  const uint16_t ioAddr = Pins.ioRequestAddress();
  const bool ioWrite = Pins.ioRequestWrite();
  if (Mc6850.isSelected(ioAddr)) {
    if (ioWrite) Mc6850.write(Pins.ioGetData(), ioAddr);
    else Pins.ioSetData(Mc6850.read(ioAddr));
  } else {
    if (ioWrite) data = Pins.ioGetData();
    else Pins.ioSetData(data);
  }

  Pins.acknowledgeIoRequest();
  Pins.leaveIoRequest();
}

void Pins::loop() {
  Mc6850.loop();
}

#define PINM(name)   _BV(__PIN__(name))
#define pinMode(name, mode) do {                        \
    if (mode == INPUT) DDR(name) &= ~PINM(name);        \
    if (mode == INPUT_PULLUP) DDR(name) &= ~PINM(name); \
    if (mode == INPUT_PULLUP) PORT(name) |= PINM(name); \
    if (mode == OUTPUT) DDR(name) |= PINM(name);        \
  } while (0)
#define digitalRead(name) (PIN(name) & PINM(name))
#define digitalWrite(name, val) do {            \
    if (val == LOW) PORT(name) &= ~PINM(name);  \
    if (val == HIGH) PORT(name) |= PINM(name);  \
  } while (0)

static inline void assertReset() {
  digitalWrite(RESET, LOW);
}
static inline void negateReset() {
  digitalWrite(RESET, HIGH);
}

static inline void assertHalt() {
  digitalWrite(HALT, LOW);
}
static inline void negateHalt() {
  digitalWrite(HALT, HIGH);
}

static inline bool isReadDirection() {
  return digitalRead(RD_WR) == HIGH;
}
static inline bool isWriteDirection() {
  return digitalRead(RD_WR) == LOW;
}

static inline void enableStep() {
  digitalWrite(STEP, LOW);
}
static inline void disableStep() {
  digitalWrite(STEP, HIGH);
}

static inline bool isIntAsserted() {
  return digitalRead(INT) == LOW;
}

static inline void assertAck() {
  digitalWrite(ACK, LOW);
}
static inline void negateAck() {
  digitalWrite(ACK, HIGH);
}

static inline void enableRam() {
  digitalWrite(RAM_E, LOW);
}
static inline void disableRam() {
  digitalWrite(RAM_E, HIGH);
}

static inline void turnOnUserLed() {
  digitalWrite(USR_LED, LOW);
}
static inline void turnOffUserLed() {
  digitalWrite(USR_LED, HIGH);
}

uint8_t Pins::Dbus::getDbus() {
  return PIN(DB);
}

void Pins::Dbus::begin() {
  setDbus(INPUT, 0);
}

void Pins::Dbus::setDbus(uint8_t dir, uint8_t data) {
  if (dir == OUTPUT && isWriteDirection()) {
    Console.println(F("!! R/W is LOW"));
    return;
  }
  if (dir == OUTPUT || _capture) {
    disableRam();
  } else {
    enableRam();
  }
  _dir = dir;
  if (dir == INPUT) {
    DDR(DB) = 0x00;
  } else {
    DDR(DB) = 0xFF;
    PORT(DB) = data;
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
  if (digitalRead(BA))    p |= Status::ba;
  if (digitalRead(BS))    p |= Status::bs;
  if (digitalRead(RESET)) p |= Status::reset;
  if (digitalRead(HALT))  p |= Status::halt;
  if (digitalRead(LIC))   p |= Status::lic;
  if (digitalRead(AVMA))  p |= Status::avma;
  if (digitalRead(RD_WR)) p |= Status::rw;
  _pins = p;
  _dbus  = Dbus::getDbus();
}

void Pins::Status::print() const {
  char buffer[16];
  char *p = buffer;
  *p++ = (_pins & lic)  ? 'L' : ' ';
  *p++ = (_pins & avma) ? 'A' : ' ';
  *p++ = (_pins & rw)   ? 'R' : 'W';
  *p++ = (_pins & halt) ? ' ' : 'H';
  *p++ = static_cast<uint8_t>(_pins & babs) + '0';
  *p++ = (_pins & reset) ? ' ' : 'R';
  p = outText(p, " DB=0x");
  p = outHex8(p, _dbus);
  Console.print(buffer);
}

void Pins::print() const {
  _signals.print();
  char buffer[4];
  char *p = buffer;
  *p++ = ' ';
  if (_signals.fetchingVector()) {
    *p++ = 'V';
  } else if (_signals.running()) {
    if (_signals.writeCycle(_previous)) {
      *p++ = 'W';
    } else if (_signals.readCycle(_previous)) {
      *p++ = 'R';
    } else {
      *p++ = '-';
    }
  } else {
    *p++ = 'H';
  }
  *p = 0;
  Console.println(buffer);
}

void Pins::reset(bool show) {
  assertReset();
  negateHalt();
  delayMicroseconds(10);
  enableStep();
  negateReset();
  do {
    cycle();
    assertHalt();
    if (show) print();
  } while (!_signals.halting());
  disableStep();
}

void Pins::cycle() {
  _previous = _signals;
  _signals.get();
  if (_signals.running()) {
    if (_signals.writeCycle(_previous)) {
      _dbus.input();
    } else if (_signals.readCycle(_previous)) {
      _dbus.output();
    }
  } else {
    _dbus.input();
  }
  _signals.get();
  assertAck();
  delayMicroseconds(4);
  _dbus.input();
  negateAck();
  delayMicroseconds(4);
}

void Pins::run() {
  negateHalt();
  disableStep();
  turnOnUserLed();
  _freeRunning = true;
  attachInterrupt(INT_INTERRUPT, ioRequest, FALLING);
}

void Pins::halt(bool show) {
  detachInterrupt(INT_INTERRUPT);
  enableStep();
  delayMicroseconds(10);
  assertHalt();
  do {
    cycle();
    if (show) print();
  } while (_signals.running());
  disableStep();
  turnOffUserLed();
  _freeRunning = false;
}

void Pins::setData(uint8_t data) {
  _dbus.set(data);
}

void Pins::unhalt() {
  enableStep();
  delayMicroseconds(10);
  negateHalt();
  do {
    cycle();
    assertHalt();
  } while (!_signals.running() || !_signals.lastInstructionCycle());
}

void Pins::execInst(const uint8_t *inst, uint8_t len, bool show) {
  unhalt();
  for (uint8_t i = 0; i < len; i++) {
    setData(inst[i]);
    cycle();
    if (show) print();
  }
  while (!_signals.lastInstructionCycle()) {
    cycle();
    if (show) print();
  }
  disableStep();
}

void Pins::captureWrites(const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max) {
  unhalt();
  for (uint8_t i = 0; i < len; i++) {
    setData(inst[i]);
    cycle();
  }
  _dbus.capture(true);
  uint8_t i = 0;
  while (!_signals.lastInstructionCycle()) {
    cycle();
    if (_signals.writeCycle(_previous) && i < max) {
      buf[i++] = _signals.dbus();
    }
  }
  _dbus.capture(false);
  disableStep();
}

void Pins::step(bool show) {
  unhalt();
  do {
    cycle();
    if (show) print();
  } while (!_signals.lastInstructionCycle());
  disableStep();
}

void Pins::begin() {
  pinMode(RESET, OUTPUT);
  assertReset();
  pinMode(HALT,  OUTPUT);
  assertHalt();
  pinMode(IRQ, OUTPUT);
  negateIrq();

  pinMode(STEP, OUTPUT);
  pinMode(ACK, OUTPUT);
  pinMode(INT, INPUT_PULLUP);
  enableStep();
  negateAck();

  pinMode(BS,    INPUT);
  pinMode(BA,    INPUT);
  pinMode(LIC,   INPUT);
  pinMode(AVMA,  INPUT);
  pinMode(RD_WR, INPUT_PULLUP);
  pinMode(RAM_E, OUTPUT);
  disableRam();

  pinMode(ADR0, INPUT_PULLUP);
  pinMode(ADR1, INPUT_PULLUP);

  pinMode(USR_SW, INPUT_PULLUP);
  pinMode(USR_LED, OUTPUT);
  turnOffUserLed();

  _dbus.begin();

  _previous.get();

  reset();
}

void Pins::assertIrq(uint8_t mask) {
  _irq |= mask;
  if (_irq)
    digitalWrite(IRQ, LOW);
}

void Pins::negateIrq(uint8_t mask) {
  _irq &= ~mask;
  if (_irq == 0)
    digitalWrite(IRQ, HIGH);
}

uint16_t Pins::ioRequestAddress() const {
  uint16_t addr = ioBaseAddress();
  if (digitalRead(ADR0)) addr |= 0x01;
  if (digitalRead(ADR1)) addr |= 0x02;
  return addr;
}

bool Pins::ioRequestWrite() const {
  return isWriteDirection();
}

uint8_t Pins::ioGetData() {
  _dbus.input();
  return Dbus::getDbus();
}

void Pins::ioSetData(uint8_t data) {
  _dbus.set(data);
  _dbus.output();
}

void Pins::acknowledgeIoRequest() {
  assertAck();
}

void Pins::leaveIoRequest() {
  _dbus.input();
  negateAck();
}

void Pins::attachUserSwitch(void (*isr)()) const {
  attachInterrupt(USR_SW_INTERRUPT, isr, FALLING);
}
