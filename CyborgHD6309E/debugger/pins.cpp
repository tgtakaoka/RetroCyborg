/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "pins.h"

#include <Arduino.h>
#include <SPI.h>
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

static inline bool validBusCycle(const Signals *prev) {
    return digitalRead(BA) == LOW && digitalRead(BS) == LOW && prev &&
           prev->advancedValidMemoryAddress();
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

static inline void assertConsoleRts() {
    digitalWrite(DBG_RTS, LOW);
}

static inline void negateConsoleRts() {
    digitalWrite(DBG_RTS, HIGH);
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
    uint8_t p = 0;
    if (digitalRead(BA) == HIGH)
        p |= ba;
    if (digitalRead(BS) == HIGH)
        p |= bs;
    if (digitalRead(RESET) == LOW)
        p |= reset;
    if (digitalRead(HALT) == LOW)
        p |= halt;
    if (digitalRead(LIC) == HIGH)
        p |= lic;
    if (digitalRead(AVMA) == HIGH)
        p |= avma;
    if (digitalRead(RD_WR) == HIGH)
        p |= rw;
    _pins = p;
    _dbus = busRead(DB);
#ifdef DEBUG_SIGNALS
    _debug = 0;
    digitalToggle(USR_LED);
#endif
}

static char *outPin(char *p, uint8_t value, const char *name) {
    if (value)
        return outText(p, name);
    for (const char *s = name; *s; s++)
        *p++ = ' ';
    *p = 0;
    return p;
}

void Signals::print(const Signals *prev) const {
    char buffer[40];
    char *p = buffer;
#ifdef DEBUG_SIGNALS
    *p++ = _debug ? _debug : ' ';
#endif
    p = outPin(p, _pins & halt, " HALT");
    p = outPin(p, _pins & ba, " BA");
    p = outPin(p, _pins & bs, " BS");
    p = outPin(p, _pins & lic, " LIC");
    p = outPin(p, _pins & avma, " AVMA");
    p = outText(p, (_pins & rw) ? " RD" : " WR");
    p = outText(p, " DB=0x");
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

void Pins::reset(bool show) {
    _cycles = 0;

    // Assert RESET and feed several clocks.
    assertReset();
    negateHalt();
    if (isIntAsserted()) {
        // restart clock generator.
        disableStep();
        while (!isIntAsserted())
            ;
        assertAck();
        delayMicroseconds(2);
        negateAck();
    }
    delayMicroseconds(10);

    // Enable STEP to Q=H E=H
    enableStep();
    while (!isIntAsserted())
        ;

    negateReset();
    assertHalt();
    for (;;) {
        Signals &signals = cycle();
        signals.debug('R');
        if (signals.halting())
            break;
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
    negateAck();

    _freeRunning = false;
    _stopRunning = false;
    if (show)
        printCycles();
}

Signals &Pins::cycle() {
    const Signals *prev = (_cycles == 0) ? nullptr : &_signals[_cycles - 1];
    Signals &signals = _signals[_cycles];

    // Advance clock to Q=L E=H
    disableStep();
    while (isIntAsserted())
        ;
    // Setup data bus
    if (validBusCycle(prev) && !isWriteDirection()) {
        _dbus.output();
    } else {
        _dbus.input();
    }
    // Get signal status while Q=L E=H
    signals.get();
    // Assert ACK to advance clock to Q=L E=L.
    assertAck();
    // Cleanup data bus.
    _dbus.input();

    if (_cycles < MAX_CYCLES)
        _cycles++;
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
    _cycles = 0;

    enableStep();
    while (!isIntAsserted())
        ;
    assertHalt();
    for (;;) {
        Signals &signals = cycle();
        signals.debug('H');
        if (signals.halting())
            break;
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
    negateAck();
    turnOffUserLed();

    if (show)
        printCycles();
}

void Pins::setData(uint8_t data) {
    _dbus.set(data);
}

Signals &Pins::unhalt() {
    enableStep();
    while (!isIntAsserted())
        ;
    negateHalt();
    cycle().debug('U');
    enableStep();
    while (isIntAsserted())
        ;
    assertHalt();
    char c = '1';
    for (;;) {
        Signals &signals = cycle();
        signals.debug(c++);
        if (signals.running() && signals.lastInstructionCycle())
            return signals;
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
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
    _cycles = 0;
    const Signals *prev = nullptr;

    unhalt();
    enableStep();
    negateAck();
    while (!isIntAsserted())
        ;
    for (uint8_t i = 0; i < len; i++) {
        setData(inst[i]);
        Signals &signals = cycle();
        signals.debug('A' + i);
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
        prev = &signals;
    }

    uint8_t cap = 0;
    if (capWrite)
        _dbus.capture(true);
    for (;;) {
        Signals &signals = cycle();
        signals.debug('a' + cap);
        if ((capWrite && signals.writeCycle(prev)) ||
                (capRead && signals.readCycle(prev))) {
            if (cap < max)
                capBuf[cap++] = signals.dbus();
        }
        if (signals.halting())
            break;
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
        prev = &signals;
    }
    if (capWrite)
        _dbus.capture(false);
    negateAck();

    if (show)
        printCycles();
    return cap;
}

void Pins::step(bool show) {
    _cycles = 0;

    unhalt();
    enableStep();
    negateAck();
    while (!isIntAsserted())
        ;
    for (;;) {
        Signals &signals = cycle();
        signals.debug('S');
        if (signals.halting())
            break;
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
    negateAck();

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

    pinMode(STEP, OUTPUT);
    pinMode(ACK, OUTPUT);
    pinMode(INT, INPUT_PULLUP);
    while (isIntAsserted()) {
        disableStep();
        assertAck();
    }
    negateAck();

    pinMode(BS, INPUT);
    pinMode(BA, INPUT);
    pinMode(LIC, INPUT);
    pinMode(AVMA, INPUT);
    pinMode(RD_WR, INPUT_PULLUP);
    pinMode(RAM_E, OUTPUT);
    disableRam();

    pinMode(ADR0, INPUT_PULLUP);
    pinMode(ADR1, INPUT_PULLUP);

    pinMode(DBG_RTS, OUTPUT);
    pinMode(USR_SW, INPUT_PULLUP);
    pinMode(USR_LED, OUTPUT);
    assertConsoleRts();
    turnOffUserLed();

    _dbus.begin();

    Console.begin(CONSOLE_BAUD);
    Cli.begin(Console);

    SPI.swap(SPI_MAPPING);

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

int Pins::sdCardChipSelectPin() const {
    return SD_CS_PIN;
}
