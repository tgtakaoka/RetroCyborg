#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

uint8_t Signals::_cycles;
Signals Signals::_signals[MAX_CYCLES + 1];

void Signals::printCycles() {
    for (uint8_t i = 0; i < _cycles; i++) {
        _signals[i].print();
    }
}

Signals &Signals::currCycle() {
    return _signals[_cycles];
}

Signals &Signals::resetCycles() {
    _cycles = 0;
    return _signals[_cycles].clear();
}

Signals &Signals::nextCycle() {
    if (_cycles < MAX_CYCLES)
        _cycles++;
    return _signals[_cycles].clear();
}

extern uint8_t RAM[0x10000];

uint8_t Signals::flushCycles(uint8_t start) {
    for (uint8_t c = start; c < _cycles; c++) {
        const Signals &signals = _signals[c];
        if (signals.rw == LOW) {
            RAM[signals.data] = signals.data;
        }
    }
    return _cycles;
}

Signals &Signals::get() {
    rw = digitalReadFast(PIN_RW);
    reset = digitalReadFast(PIN_RESET);
    e = digitalReadFast(PIN_E);
    return *this;
}

Signals &Signals::readAddr() {
    as = digitalReadFast(PIN_AS);
    if (as == HIGH) {
        addr = busRead(AH);
        addr <<= 8;
        addr |= busRead(DB);
    }
    return *this;
}

Signals &Signals::readData() {
    data = busRead(DB);
    return *this;
}

Signals &Signals::clear() {
    addr = 0;
    data = 0;
    as = rw = reset = e = 0;
    _inject = _capture = false;
    return *this;
}

Signals &Signals::inject(uint8_t val) {
    _inject = true;
    data = val;
    return *this;
}

Signals &Signals::capture() {
    _capture = true;
    return *this;
}

static char *outPin(char *p, bool value, const char *name) {
    if (value)
        return outText(p, name);
    while (*name++)
        *p++ = ' ';
    *p = 0;
    return p;
}

void Signals::print() const {
    // text=17 hex=(1+2)*2 eos=1
    char buffer[24];
    char *p = buffer;
#ifdef DEBUG_SIGNALS
    *p++ = _debug ? _debug : ' ';
#endif
    p = outPin(p, reset == LOW, " R");
    p = outPin(p, e == HIGH, " E");
    p = outPin(p, as == HIGH, " AS");
    p = outText(p, rw == HIGH ? " RD" : " WR");
    p = outText(p, " A=");
    p = outHex16(p, addr);
    p = outText(p, " D=");
    p = outHex8(p, data);
    *p = 0;
    libcli::Cli::instance().println(buffer);
}

Signals &Signals::debug(char c) {
#ifdef DEBUG_SIGNALS
    _debug = c;
#endif
    return *this;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4: