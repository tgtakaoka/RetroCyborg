/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "pins.h"

#include <avr/io.h>
#include <libcli.h>

#include "commands.h"
#include "mc6850.h"
#include "pins_base.h"
#include "pins_map.h"
#include "string_util.h"

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

static inline bool userSwitchAsserted() {
    return digitalRead(USR_SW) == LOW;
}
static inline void turnOnUserLed() {
    digitalWrite(USR_LED, LOW);
}
static inline void turnOffUserLed() {
    digitalWrite(USR_LED, HIGH);
}

extern class libcli::Cli Cli;
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

uint8_t Pins::Dbus::getDbus() {
    return busRead(DB);
}

void Pins::Dbus::begin() {
    setDbus(INPUT, 0);
}

void Pins::Dbus::setDbus(uint8_t dir, uint8_t data) {
    if (dir == OUTPUT && isWriteDirection()) {
        Cli.println(F("!! R/W is LOW"));
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

void Pins::Status::get() {
    uint8_t p = 0;
    if (digitalRead(BA))
        p |= Status::ba;
    if (digitalRead(BS))
        p |= Status::bs;
    if (digitalRead(RESET))
        p |= Status::reset;
    if (digitalRead(HALT))
        p |= Status::halt;
    if (digitalRead(LIC))
        p |= Status::lic;
    if (digitalRead(AVMA))
        p |= Status::avma;
    if (digitalRead(RD_WR))
        p |= Status::rw;
    _pins = p;
    _dbus = Dbus::getDbus();
}

void Pins::Status::print() const {
    char buffer[32];
    char *p = buffer;
    p = outText(p, (_pins & halt) ? F(" HALT") : F("     "));
    p = outText(p, (_pins & ba) ? F(" BA") : F("   "));
    p = outText(p, (_pins & bs) ? F(" BS") : F("   "));
    p = outText(p, (_pins & lic) ? F(" LIC") : F("    "));
    p = outText(p, (_pins & avma) ? F(" AVMA") : F("     "));
    p = outText(p, (_pins & rw) ? F(" RD") : F(" WR"));
    p = outText(p, F(" DB=0x"));
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
    if (!_signals.lastInstructionCycle()) {
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
    }
    negateAck();
}

void Pins::captureWrites(
        const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max) {
    unhalt();
    enableStep();
    negateAck();
    while (!isIntAsserted())
        ;
    for (uint8_t i = 0; i < len; i++) {
        setData(inst[i]);
        cycle();
        enableStep();
        negateAck();
        while (!isIntAsserted())
            ;
    }
    if (!_signals.lastInstructionCycle()) {
        _dbus.capture(true);
        uint8_t i = 0;
        for (;;) {
            cycle();
            if (_signals.writeCycle(_previous) && i < max) {
                buf[i++] = _signals.dbus();
            }
            if (_signals.lastInstructionCycle())
                break;
            enableStep();
            negateAck();
            while (!isIntAsserted())
                ;
        }
        _dbus.capture(false);
    }
    negateAck();
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
    assertReset();
    pinMode(RESET, OUTPUT);
    negateHalt();
    pinMode(HALT, OUTPUT);
    negateIrq();
    pinMode(IRQ, OUTPUT);

    disableStep();
    negateAck();
    pinMode(STEP, OUTPUT);
    pinMode(ACK, OUTPUT);
    pinMode(INT, INPUT_PULLUP);

    pinMode(BS, INPUT);
    pinMode(BA, INPUT);
    pinMode(LIC, INPUT);
    pinMode(AVMA, INPUT);
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

    Console.begin(CONSOLE_BAUD);
    Cli.begin(Console);

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
    if (digitalRead(ADR0))
        addr |= 0x01;
    if (digitalRead(ADR1))
        addr |= 0x02;
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
    while (isIntAsserted())
        ;
}

void Pins::leaveIoRequest() {
    _dbus.input();
    negateAck();
}
