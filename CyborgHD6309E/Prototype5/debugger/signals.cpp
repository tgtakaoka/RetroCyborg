#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "string_util.h"

extern libcli::Cli &cli;

uint8_t Signals::_cycles;
Signals Signals::_signals[MAX_CYCLES + 1];

void Signals::printCycles() {
    const Signals *prev = nullptr;
    for (uint8_t i = 0; i < _cycles; i++) {
        _signals[i].print(prev);
        prev = &_signals[i];
    }
}

Signals &Signals::currCycle() {
    return _signals[_cycles];
}

void Signals::resetCycles() {
    _cycles = 0;
}

void Signals::nextCycle() {
    if (_cycles < MAX_CYCLES)
        _cycles++;
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
    _debug = 0;
}

static void outPin(char *p, bool value, const __FlashStringHelper *name) {
    if (value) {
        outText(p, name);
        return;
    }
    for (PGM_P s = reinterpret_cast<PGM_P>(name); pgm_read_byte(s); s++)
        *p++ = ' ';
}

void Signals::print(const Signals *prev) const {
    static char buffer[] = {
        'D', ' ',                // _debug=0
        'B', 'A', ' ',           // ba=2
        'B', 'S', ' ',           // bs=5
        'B', 'U', 'S', 'Y', ' ', // busy=8
        'L', 'I', 'C', ' ',      // lic=13
        'A', 'V', 'M', 'A', ' ', // avma=17
        'R', 'D', ' ',           // r/w=22
        'D', '=', 0, 0,          // _dbus=27
        ' ',                     // status?=29
        'S', 0                   // status=30
    };
    buffer[0] = _debug ? _debug : ' ';
    outPin(buffer + 2, _pins & ba, F("BA"));
    outPin(buffer + 5, _pins & bs, F("BS"));
    outPin(buffer + 8, _pins & busy, F("BUSY"));
    outPin(buffer + 13, _pins & lic, F("LIC"));
    outPin(buffer + 17, _pins & avma, F("AVMA"));
    outText(buffer + 22, (_pins & rw) ? F("RD") : F("WR"));
    outHex8(buffer + 27, _dbus);
    if (prev) {
        char status;
        if (fetchingVector()) {
            status = 'V';
        } else if (running()) {
            if (writeCycle(prev)) {
                status = 'W';
            } else if (readCycle(prev)) {
                status = 'R';
            } else {
                status = '-';
            }
        } else if (halting()) {
            status = 'H';
        } else {
            status = 'S';
        }
        buffer[29] = ' ';
        buffer[30] = status;
    } else {
        buffer[29] = 0;
    }
    cli.println(buffer);
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
