#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "string_util.h"

extern libcli::Cli &cli;

uint8_t Signals::_put = 0;
uint8_t Signals::_get = 0;
uint8_t Signals::_cycles = 0;
Signals Signals::_signals[MAX_CYCLES];

void Signals::printCycles() {
    const Signals *prev = nullptr;
    for (auto i = 0; i < _cycles; i++) {
        const auto idx = (_get + i) % MAX_CYCLES;
        _signals[idx].print(prev);
        prev = &_signals[idx];
    }
}

Signals &Signals::currCycle() {
    return _signals[_put];
}

void Signals::resetCycles() {
    _cycles = 0;
    _signals[_get = _put];
}

void Signals::nextCycle() {
    _put++;
    _put %= MAX_CYCLES;
    if (_cycles < MAX_CYCLES) {
        _cycles++;
    } else {
        _get++;
        _get %= MAX_CYCLES;
    }
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

void Signals::print(const Signals *prev) const {
    // clang-format off
    static char buffer[] = {
        'D', ' ',               // _debug=0
        'V',                    // ba/bs=2
        'A',                    // avma=3
        'B',                    // busy=4
        'L',                    // lic=5
        'W', ' ',               // rw=6
        'D', '=', 0, 0,         // _dbus=10
        ' ',                    // status?=12
        'S', 0                  // status=13
    };
    // clang-format on
    buffer[0] = _debug ? _debug : ' ';
    char *p = buffer + 2;
    if ((_pins & ba) == 0) {
        *p++ = (_pins & bs) == 0 ? ' ' : 'V';
    } else {
        *p++ = (_pins & bs) == 0 ? 'S' : 'H';
    }
    *p++ = (_pins & avma) == 0 ? ' ' : 'A';
    *p++ = (_pins & busy) == 0 ? ' ' : 'B';
    *p++ = (_pins & lic) == 0 ? ' ' : 'L';
    *p++ = (_pins & rw) == 0 ? 'W' : 'R';
    outHex8(buffer + 10, _dbus);
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
        buffer[12] = ' ';
        buffer[13] = status;
    } else {
        buffer[12] = 0;
    }
    cli.println(buffer);
}

void Signals::printSignals() {
    Signals s;
    s.get();
    // clang-format off
    static char buffer[] = {
        'R',                    // RESET=0
        'H',                    // HALT=1
        ' ',
        0,
    };
    // clang-format on
    buffer[0] = s.resetAsserted() ? 'R' : ' ';
    buffer[1] = s.haltAsserted() ? 'H' : ' ';
    cli.print(buffer);
    s.print();
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
