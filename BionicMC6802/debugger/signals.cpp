#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli &cli;

uint8_t Signals::_cycles;
Signals Signals::_signals[MAX_CYCLES + 1];

Signals &Signals::clear() {
    addr = 0;
    data = 0;
    vma = rw = ba = halt = 0;
#ifdef DEBUG_SIGNALS
    _debug = 0;
#endif
    _inject = _capture = false;
    return *this;
}

void Signals::printCycles(const Signals *end) {
    if (end == nullptr)
        end = &currCycle();
    for (const auto *signals = _signals; signals < end; signals++) {
        signals->print();
        Pins.idle();
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

void Signals::flushWrites(const Signals *end) {
    for (const auto *signals = _signals; signals < end; signals++) {
        if (signals->rw == LOW)
            Memory.write(signals->addr, signals->data);
    }
}

Signals &Signals::get() {
    rw = digitalReadFast(PIN_RW);
    ba = digitalReadFast(PIN_BA);
    halt = digitalReadFast(PIN_HALT);
    return *this;
}

Signals &Signals::readAddr() {
    vma = digitalReadFast(PIN_VMA);
    if (vma == HIGH) {
        addr = busRead(AL);
#if defined(AM_vp)
        addr |= busRead(AM);
#endif
        addr |= busRead(AH);
    }
    return *this;
}

Signals &Signals::readData() {
    data = busRead(D);
    return *this;
}

Signals &Signals::inject(uint8_t val) {
    Signals &curr = currCycle();
    curr._inject = true;
    curr.data = val;
    return curr;
}

Signals &Signals::capture() {
    Signals &curr = currCycle();
    curr._capture = true;
    return curr;
}

static char *outPin(char *p, bool value, const char *name)
        __attribute((unused));
static char *outPin(char *p, bool value, const char *name) {
    if (value)
        return outText(p, name);
    while (*name++)
        *p++ = ' ';
    *p = 0;
    return p;
}

void Signals::print() const {
    const auto debug = 2;
    const auto text = 13;
    const auto hex = (1 + 2) * 2;
    const auto eos = 1;
    static char buffer[debug + text + hex + eos];
    char *p = buffer;
#ifdef DEBUG_SIGNALS
    *p++ = _debug ? _debug : ' ';
    *p++ = ' ';
#endif
    *p++ = halt == LOW ? 'H' : ' ';
    p = outPin(p, ba == HIGH, " BA ");
    *p++ = vma == LOW ? ' ' : 'V';
    *p++ = rw == LOW ? 'W' : 'R';
    p = outText(p, " A=");
    p = outHex16(p, addr);
    p = outText(p, " D=");
    p = outHex8(p, data);
    *p = 0;
    cli.println(buffer);
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
