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
    _pins = p;
    _dbus = busRead(DB);
#ifdef DEBUG_SIGNALS
    _debug = 0;
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

void Signals::print() const {
    char buffer[34];
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
    Cli.print(buffer);
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
    Cli.println(buffer);
}

void Pins::reset(bool show) {
    // Assert RESET and feed several clocks.
    assertReset();
    negateHalt();
    if (isIntAsserted()) {
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
        cycle();
        if (show)
            print();
        if (_signals.halting())
            break;
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
    negateAck();

    _freeRunning = false;
    _stopRunning = false;
}

void Pins::cycle() {
    _previous = _signals;
    // Advance clock to Q=L E=H
    disableStep();
    while (isIntAsserted())
        ;
    // Get signal status while Q=L E=H
    _signals.get();
    // Setup data bus
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
    // Assert ACK to advance clock to Q=L E=L.
    assertAck();
    // Cleanup data bus.
    _dbus.input();
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
    enableStep();
    while (!isIntAsserted())
        ;
    assertHalt();
    for (;;) {
        cycle();
        if (show)
            print();
        if (_signals.halting())
            break;
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
    negateAck();
    turnOffUserLed();
}

void Pins::setData(uint8_t data) {
    _dbus.set(data);
}

void Pins::unhalt() {
    enableStep();
    while (!isIntAsserted())
        ;
    negateHalt();
    cycle();
    enableStep();
    while (isIntAsserted())
        ;
    assertHalt();
    for (;;) {
        cycle();
        if (_signals.running() && _signals.lastInstructionCycle())
            break;
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
    unhalt();
    enableStep();
    negateAck();
    while (!isIntAsserted())
        ;
    for (uint8_t i = 0; i < len; i++) {
        setData(inst[i]);
        cycle();
        if (show)
            print();
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
    uint8_t cap = 0;
    if (!_signals.lastInstructionCycle()) {
        if (capWrite)
            _dbus.capture(true);
        for (;;) {
            cycle();
            if (show)
                print();
            if ((capWrite && _signals.writeCycle(_previous)) ||
                    (capRead && _signals.readCycle(_previous))) {
                if (cap < max)
                    capBuf[cap++] = _signals.dbus();
            }
            if (_signals.lastInstructionCycle())
                break;
            enableStep();
            negateAck();
            while (!isIntAsserted())
                ;
        }
        if (capWrite)
            _dbus.capture(false);
    }
    negateAck();
    return cap;
}

void Pins::step(bool show) {
    unhalt();
    enableStep();
    negateAck();
    while (!isIntAsserted())
        ;
    for (;;) {
        cycle();
        if (show)
            print();
        if (_signals.lastInstructionCycle())
            break;
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
    negateAck();
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

    _previous.get();

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
