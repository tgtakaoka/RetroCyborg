/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "pins.h"

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <libcli.h>

#include "commands.h"
#include "mc6850.h"
#include "pins_base.h"
#include "pins_map.h"
#include "string_util.h"

extern libcli::Cli Cli;

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

static inline void assertNmi() {
    digitalWrite(NMI, LOW);
}

static inline void negateNmi() {
    digitalWrite(NMI, HIGH);
}

static inline bool isWriteDirection() {
    return digitalRead(RD_WR) == LOW;
}

static inline bool isUnhalt() {
#ifdef SIGNALS_BUS
    const uint8_t pins = busRead(SIGNALS);
    return (pins & (_BV(BA_PIN) | _BV(BS_PIN) | _BV(LIC_PIN))) == 0;
#else
    return digitalRead(BA) == LOW && digitalRead(BS) == LOW &&
           digitalRead(LIC) == LOW;
#endif
}

static inline bool validBusCycle(const Signals *prev) {
#ifdef SIGNALS_BUS
    const uint8_t pins = busRead(SIGNALS);
    return (pins & (_BV(BA_PIN) | _BV(BS_PIN))) == 0 && prev &&
           prev->advancedValidMemoryAddress();
#else
    return digitalRead(BA) == LOW && digitalRead(BS) == LOW && prev &&
           prev->advancedValidMemoryAddress();
#endif
}

static inline void enableStep() {
    digitalWrite(STEP, LOW);
}

static inline void disableStep() {
    digitalWrite(STEP, HIGH);
}

static inline bool isStepEnabled() {
    return digitalRead(STEP) == LOW;
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

static inline bool userSwitchAsserted() {
    return digitalRead(USR_SW) == LOW;
}

static inline void turnOnUserLed() {
    digitalWrite(USR_LED, HIGH);
}

static inline void turnOffUserLed() {
    digitalWrite(USR_LED, LOW);
}

static void toggleUserLed() __attribute__((unused));
static inline void toggleUserLed() {
    if (digitalRead(USR_LED)) {
        digitalWrite(USR_LED, LOW);
    } else {
        digitalWrite(USR_LED, HIGH);
    }
}

class Pins Pins;

class Mc6850 Mc6850(Console, Pins::ioBaseAddress(),
        Pins::getIrqMask(Pins::ioBaseAddress()),
        Pins::getIrqMask(Pins::ioBaseAddress() + 1));

static uint8_t data;

static void ioRequest() {
    const uint16_t ioAddr = Pins.ioRequestAddress();
    const bool ioWrite = Pins.ioRequestWrite();
    if (Mc6850.isSelected(ioAddr)) {
        if (ioWrite)
            Mc6850.write(Pins.ioGetData(), ioAddr);
        else
            Pins.ioSetData(Mc6850.read(ioAddr));
    } else {
        if (ioWrite)
            data = Pins.ioGetData();
        else
            Pins.ioSetData(data);
    }

    if (userSwitchAsserted()) {
        enableStep();
        detachInterrupt(INT_INTERRUPT);
        Pins.stopRunning();
    }

    Pins.acknowledgeIoRequest();
    Pins.leaveIoRequest();
}

void Pins::Dbus::begin() {
    setDbus(INPUT, 0);
}

void Pins::Dbus::setDbus(uint8_t dir, uint8_t data) {
    if (dir == OUTPUT && isWriteDirection()) {
        Cli.println("!! R/W is LOW");
        return;
    }
    if (dir == OUTPUT || _capture) {
        disableRam();
    } else {
        enableRam();
    }
    _dir = dir;
    if (dir == INPUT) {
        busMode(DB, INPUT);
    } else {
        busWrite(DB, data);
        busMode(DB, OUTPUT);
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

void Signals::get() {
#if defined(SIGNALS_BUS)
    _pins = busRead(SIGNALS);
#else
    uint8_t p = 0;
    if (digitalRead(BA) == HIGH)
        p |= ba;
    if (digitalRead(BS) == HIGH)
        p |= bs;
    if (digitalRead(RESET) == HIGH)
        p |= reset;
    if (digitalRead(HALT) == HIGH)
        p |= halt;
    if (digitalRead(LIC) == HIGH)
        p |= lic;
    if (digitalRead(AVMA) == HIGH)
        p |= avma;
    if (digitalRead(RD_WR) == HIGH)
        p |= rw;
    if (digitalRead(BUSY) == HIGH)
        p |= busy;
    _pins = p;
#endif
    _dbus = busRead(DB);
#ifdef DEBUG_SIGNALS
    _debug = 0;
    digitalToggle(USR_LED);
#endif
}

static char *outPin(char *p, bool value, const __FlashStringHelper *name) {
    if (value)
        return outText(p, name);
    for (PGM_P s = reinterpret_cast<PGM_P>(name); pgm_read_byte(s); s++)
        *p++ = ' ';
    *p = 0;
    return p;
}

void Signals::print(const Signals *prev) const {
    char buffer[45];
    char *p = buffer;
#ifdef DEBUG_SIGNALS
    *p++ = _debug ? _debug : ' ';
#endif
    p = outPin(p, (_pins & halt) == 0, F(" HALT"));
    p = outPin(p, _pins & ba, F(" BA"));
    p = outPin(p, _pins & bs, F(" BS"));
    p = outPin(p, _pins & busy, F(" BUSY"));
    p = outPin(p, _pins & lic, F(" LIC"));
    p = outPin(p, _pins & avma, F(" AVMA"));
    p = outText(p, (_pins & rw) ? F(" RD") : F(" WR"));
    p = outText(p, F(" DB=0x"));
    p = outHex8(p, _dbus);
    if (prev) {
        *p++ = ' ';
        if (fetchingVector()) {
            *p++ = 'V';
        } else if (running()) {
            if (writeCycle(prev)) {
                *p++ = 'W';
            } else if (readCycle(prev)) {
                *p++ = 'R';
            } else {
                *p++ = '-';
            }
        } else if (halting()) {
            *p++ = 'H';
        } else {
            *p++ = 'S';
        }
    }
    *p = 0;
    Cli.println(buffer);
}

void Pins::print() {
    Signals &signals = _signals[0];
    signals.get();
    signals.print();
}

void Pins::printCycles() const {
    const Signals *prev = nullptr;
    for (uint8_t i = 0; i < _cycles; i++) {
        _signals[i].print(prev);
        prev = &_signals[i];
    }
}

/*
 * Advance from Q=H E=H to Q=L E=H.
 * #RESET and #HALT get recognized.
 * LIC/AVMA get valid.
 */
static void fallingQ() {
    disableStep();
    while (isIntAsserted())
        ;
}

/*
 * Advance from Q=L E=H to Q=L H=L.
 * Data bus get read.
 */
static void fallingE() {
    assertAck();
}

/*
 * Advance from Q=L H=L to Q=H E=H.
 * R/W, BA/BS, Address/Data get valid.
 */
static void raisingE() {
    enableStep();
    negateAck();
    while (!isIntAsserted())
        ;
}

/*
 * Restart clocks from Q=H E=H.
 */
static void restartEQ() {
    fallingQ();
    fallingE();
    disableStep();
    delayMicroseconds(2);
    negateAck();
}

/*
 * Restart clock generator itself.
 */
static void restartClockGenerator() {
    if (isIntAsserted()) {
        disableStep();
        delayMicroseconds(2);
    }
    if (isIntAsserted()) {
        assertAck();
        delayMicroseconds(2);
        negateAck();
    }
}

void Pins::reset(bool show) {
    assertReset();
    negateHalt();
    restartClockGenerator();
    delayMicroseconds(10);

    resetCycles();
    const Signals *prev = nullptr;
    raisingE();
    negateReset();
    assertHalt();
    for (;;) {
        fallingQ();
        Signals &signals = cycle(prev);
        signals.debug('R');
        if (signals.halting())
            break;
        prev = &signals;
    }
    restartEQ();

    turnOffUserLed();
    _freeRunning = false;
    _stopRunning = false;
    if (show)
        printCycles();
}

/**
 * Advance one bus cycle.
 *
 * A bus cycle starts from
 *   E=H Q=L #STEP=H #ACK=H #INT=H
 * and ends at
 *   E=H Q=H #STEP=L #ACK=H #INT=L
 * When returned, clock generater is waiting for #ACK=H.
 */
Signals &Pins::cycle(const Signals *prev) {
    // Setup data bus.
    if (validBusCycle(prev) && !isWriteDirection()) {
        _dbus.output();
    } else {
        _dbus.input();
    }
    Signals &signals = currCycle();
    signals.get();
    fallingE();
    // Cleanup data bus.
    _dbus.input();
    raisingE();

    nextCycle();
    return signals;
}

void Pins::stopRunning() {
    _stopRunning = true;
}

void Pins::loop() {
    if (_freeRunning) {
        Mc6850.loop();
        if (userSwitchAsserted()) {
            enableStep();
            detachInterrupt(INT_INTERRUPT);
            stopRunning();
        }
    }

    if (_stopRunning) {
        _stopRunning = false;
        _freeRunning = false;
        halt(false);
        Commands.begin();
    }
}

void Pins::run() {
    _freeRunning = true;
    attachInterrupt(INT_INTERRUPT, ioRequest, FALLING);
    turnOnUserLed();
    negateHalt();
}

void Pins::halt(bool show) {
    resetCycles();
    const Signals *prev = nullptr;
    raisingE();
    assertHalt();
    for (;;) {
        fallingQ();
        Signals &signals = cycle(prev);
        signals.debug('H');
        if (signals.halting())
            break;
        prev = &signals;
    }
    restartEQ();

    turnOffUserLed();
    if (show)
        printCycles();
}

void Pins::setData(uint8_t data) {
    _dbus.set(data);
}

const Signals *Pins::unhalt() {
    resetCycles();
    const Signals *prev = nullptr;
    raisingE();
    negateHalt();
    fallingQ();
    assertHalt();
    for (;;) {
        if (isUnhalt())
            break;
        Signals &signals = cycle(prev);
        signals.debug('U');
        fallingQ();
        prev = &signals;
    }
    return prev;
}

void Pins::execInst(const uint8_t *inst, uint8_t len, bool show) {
    execute(inst, len, nullptr, 0, false, false, show);
}

uint8_t Pins::captureReads(
        const uint8_t *inst, uint8_t len, uint8_t *capBuf, uint8_t max) {
    return execute(inst, len, capBuf, max, false, capBuf != nullptr, false);
}

uint8_t Pins::captureWrites(
        const uint8_t *inst, uint8_t len, uint8_t *capBuf, uint8_t max) {
    return execute(inst, len, capBuf, max, capBuf != nullptr, false, false);
}

uint8_t Pins::execute(const uint8_t *inst, uint8_t len, uint8_t *capBuf,
        uint8_t max, bool capWrite, bool capRead, bool show) {
    uint8_t cap = 0;
    const Signals *prev = unhalt();
    for (uint8_t i = 0; i < len;) {
        setData(inst[i]);
        Signals &signals = cycle(prev);
        if (signals.readCycle(prev)) {
            signals.debug('A' + i);
            i++;
        } else {
            signals.debug('-');
        }
        if (signals.halting())
            goto end;
        fallingQ();
        prev = &signals;
    }

    if (capWrite)
        _dbus.capture(true);
    for (;;) {
        Signals &signals = cycle(prev);
        if ((capWrite && signals.writeCycle(prev)) ||
                (capRead && signals.readCycle(prev))) {
            signals.debug('a' + cap);
            if (cap < max)
                capBuf[cap++] = signals.dbus();
        } else {
            signals.debug('-');
        }
        if (signals.halting())
            break;
        fallingQ();
        prev = &signals;
    }
    if (capWrite)
        _dbus.capture(false);
end:
    restartEQ();
    if (show)
        printCycles();
    return cap;
}

void Pins::step(bool show) {
    const Signals *prev = unhalt();
    for (;;) {
        Signals &signals = cycle(prev);
        signals.debug('S');
        if (signals.halting())
            break;
        fallingQ();
        prev = &signals;
    }
    restartEQ();
    if (show)
        printCycles();
}

void Pins::begin() {
    pinMode(RESET, OUTPUT);
    assertReset();
    pinMode(HALT, OUTPUT);
    negateHalt();
    pinMode(NMI, OUTPUT);
    negateNmi();
    pinMode(IRQ, OUTPUT);
    negateIrq();

    disableStep();
    pinMode(STEP, OUTPUT);
    negateAck();
    pinMode(ACK, OUTPUT);
    pinMode(INT, INPUT_PULLUP);
    restartClockGenerator();

    pinMode(BS, INPUT);
    pinMode(BA, INPUT);
    pinMode(LIC, INPUT);
    pinMode(AVMA, INPUT);
    pinMode(BUSY, INPUT);
    pinMode(RD_WR, INPUT_PULLUP);
    pinMode(RAM_E, OUTPUT);
    disableRam();

    _dbus.begin();

    pinMode(ADR0, INPUT_PULLUP);
    pinMode(ADR1, INPUT_PULLUP);

    pinMode(USR_SW, INPUT_PULLUP);
    pinMode(USR_LED, OUTPUT);
    turnOffUserLed();

    Console.begin(CONSOLE_BAUD);
    Cli.begin(Console);

    assertHalt();
    negateReset();
    _freeRunning = false;
    _stopRunning = false;
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
    if (digitalRead(ADR0) == HIGH)
        addr |= 0x01;
    if (digitalRead(ADR1) == HIGH)
        addr |= 0x02;
    return addr;
}

bool Pins::ioRequestWrite() const {
    return isWriteDirection();
}

uint8_t Pins::ioGetData() {
    _dbus.input();
    return busRead(DB);
}

void Pins::ioSetData(uint8_t data) {
    _dbus.set(data);
    _dbus.output();
}

void Pins::acknowledgeIoRequest() {
    assertAck();
    while (isIntAsserted())
        ;
}

void Pins::leaveIoRequest() {
    _dbus.input();
    negateAck();
}
