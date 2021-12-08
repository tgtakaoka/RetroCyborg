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
    *p++ = _debug ? _debug : ' ';
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
    cli.println(buffer);
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
