/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "pins.h"

#include <Arduino.h>
#include <libcli.h>

#include "commands.h"
#include "pins_base.h"
#include "pins_map.h"
#include "string_util.h"

extern libcli::Cli Cli;

static inline uint8_t address0() {
    return digitalRead(AD0) ? 1 : 0;
}

static inline void assertReset() {
    digitalWrite(RESET, LOW);
}

static inline void negateReset() {
    digitalWrite(RESET, HIGH);
}

static inline void negateWait() {
    // Fire event channel 4 to reset #WAIT.
    EVSYS.STROBE = EVSYS_STROBE0_EV_STROBE_CH4_gc;
}

static inline void assertBusReq() {
    digitalWrite(BUSREQ, LOW);
}

static inline void negateBusReq() {
    digitalWrite(BUSREQ, HIGH);
}

static inline void assertInt() {
    digitalWrite(INT, LOW);
}

static inline void negateInt() {
    digitalWrite(INT, HIGH);
}

static inline void assertNmi() {
    digitalWrite(NMI, LOW);
}

static inline void negateNmi() {
    digitalWrite(NMI, HIGH);
}

static inline void enableRam() {
    digitalWrite(RAMEN, HIGH);
}

static inline void disableRam() {
    digitalWrite(RAMEN, LOW);
}

static inline bool isWriteDirection() {
    return digitalRead(WR) == LOW;
}

static inline void setupCCL() {
    // #IORQ, negate input, pullup
    pinMode(IORQ, INPUT_PULLUP);
    pinModeInvert(IORQ);
    // #WAIT, negate output
    pinMode(WAIT, OUTPUT);
    pinModeInvert(WAIT);

    // Connect IORQ to CCL.LUT.IN0
    // Connect MREQ to CCL.LUT.IN1, then OR/OFF with IORQ
    CCL.LUT2CTRLB = CCL_INSEL1_MASK_gc | CCL_INSEL0_IO_gc;
    CCL.LUT2CTRLC = CCL_INSEL2_MASK_gc;
    CCL.TRUTH2 = 0xAA;  // through input0

    // Allocate Event.Channel4 to CCL.LUT3.EVENTA
    EVSYS.USERCCLLUT3A = EVSYS_CHANNEL_CHANNEL4_gc;
    EVSYS.CHANNEL4 = EVSYS_GENERATOR_OFF_gc;
    CCL.LUT3CTRLB = CCL_INSEL1_MASK_gc | CCL_INSEL0_EVENTA_gc;
    CCL.LUT3CTRLC = CCL_INSEL2_MASK_gc;
    CCL.TRUTH3 = 0xAA;  // through input0

    // Setup RS latch
    // S=IORQ, R=Event.Channel4, Q=WAIT
    CCL.SEQCTRL1 = CCL_SEQSEL1_RS_gc;

    // Start CCL
    CCL.LUT2CTRLA = CCL_ENABLE_bm | CCL_OUTEN_bm;
    CCL.LUT3CTRLA = CCL_ENABLE_bm;
    CCL.CTRLA = CCL_ENABLE_bm;
}

static inline void setupZ80Pins() {
    pinMode(RESET, OUTPUT);
    assertReset();

    pinMode(CLK, OUTPUT);
    digitalWrite(CLK, HIGH);

    // Setup #IORQ and #WAIT
    setupCCL();

    pinMode(BUSREQ, OUTPUT);
    negateBusReq();
    pinMode(INT, OUTPUT);
    negateInt();
    pinMode(NMI, OUTPUT);
    negateNmi();
    pinMode(RAMEN, OUTPUT);
    disableRam();

    pinMode(M1, INPUT_PULLUP);
    pinMode(MREQ, INPUT_PULLUP);
    pinMode(WR, INPUT_PULLUP);
    pinMode(RD, INPUT_PULLUP);
    pinMode(AD0, INPUT_PULLUP);
}

uint8_t Pins::Dbus::getDbus() {
    return busRead(DB);
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

void Pins::pulseClock(uint8_t numClocks) {
    for (uint8_t i = 0; i < numClocks; i++) {
        digitalWrite(CLK, HIGH);
        digitalWrite(CLK, LOW);
    }
}

void Pins::Status::get() {
    uint8_t p = 0;
    if (digitalRead(IORQ))
        p |= Status::iorq;  // inverted
    if (!digitalRead(MREQ))
        p |= Status::mreq;
    if (!digitalRead(M1))
        p |= Status::m1;
    if (digitalRead(WAIT))
        p |= Status::wait;  // inverted
    if (!digitalRead(RD))
        p |= Status::rd;
    if (!digitalRead(WR))
        p |= Status::wr;
    if (!digitalRead(BUSREQ))
        p |= Status::busreq;
    _pins = p;
    _dbus = Dbus::getDbus();
}

static char *outPin(char *p, uint8_t value, const char *name) {
    if (value)
        return outText(p, name);
    for (const char *s = name; *s; s++)
        *p++ = ' ';
    *p = 0;
    return p;
}

void Pins::Status::print() const {
    char buffer[5 * 4 + 3 * 3 + 6 + 2 + 2];
    char *p = buffer;
    p = outPin(p, _pins & iorq, " IORQ");
    p = outPin(p, _pins & wait, " WAIT");
    p = outPin(p, _pins & mreq, " MREQ");
    p = outPin(p, _pins & m1, " M1");
    p = outPin(p, _pins & rd, " RD");
    p = outPin(p, _pins & wr, " WR");
    p = outPin(p, _pins & busreq, " BREQ");
    p = outText(p, " DB=0x");
    p = outHex8(p, _dbus);
    Cli.print(buffer);
}

void Pins::print() const {
    _signals.print();
    Cli.println();
}

void Pins::reset(bool show) {
    assertReset();
    _signals.get();
    for (int i = 0; i < 6; i++) {
        if (show)
            print();
        cycle();
    }
    negateReset();
    _signals.get();
    for (int i = 0; i < 2; i++) {
        if (show)
            print();
        cycle();
    }
    if (show)
        print();

    _freeRunning = false;
}

void Pins::cycle() {
    pulseClock(1);
    _signals.get();
}

void Pins::loop() {}

void Pins::run() {
    _freeRunning = true;
}

void Pins::begin() {
    setupZ80Pins();

    Console.begin(CONSOLE_BAUD);
    Cli.begin(Console);

    reset();
}

class Pins Pins;
