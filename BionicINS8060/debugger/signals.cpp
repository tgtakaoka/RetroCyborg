#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli &cli;

uint8_t Signals::_cycles;
Signals Signals::_signals[MAX_CYCLES + 1];

void Signals::clear() {
    // fields including _debug will be written in Pins::cycle().
    _inject = _capture = false;
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

void Signals::resetCycles() {
    _signals[_cycles = 0].clear();
}

void Signals::nextCycle() {
    _cycles++;
    _cycles %= MAX_CYCLES;
    _signals[_cycles].clear();
}

bool Signals::getAddr() {
    if (digitalReadFast(PIN_ADS) == LOW) {
        flags = busRead(DB);
        addr = (static_cast<uint16_t>(flags & 0xF) << 12)
#if defined(ADM_vp)
            | busRead(ADM)
#endif
            | busRead(ADL);
        return true;
    }
    return false;
}

void Signals::getData() {
    data = busRead(DB);
}

void Signals::inject(uint8_t val) {
    Signals &curr = currCycle();
    curr._inject = true;
    curr.data = val;
    curr.debug('i');
}

void Signals::capture() {
    Signals &curr = currCycle();
    curr._capture = true;
    curr.debug('c');
}

void Signals::print() const {
    // clang-format off
    static char buffer[] = {
        ' ', ' ',                  // _debug=0
        ' ',                       // I/D/H=2
        'R',                       // R/W=3
        ' ', 'A', '=', 0, 0, 0, 0, // addr=7
        ' ', 'D', '=', 0, 0,       // data=14
        0,
    };
    // clang-format off
    buffer[0] = _debug;
    buffer[2] = fetchInsn() ? 'I' : (delay() ? 'D' : (halt() ? 'H' : ' '));
    buffer[3] = read() ? 'R' : 'W';
    outHex16(buffer + 7, addr);
    outHex8(buffer + 14, data);
    cli.println(buffer);
}

Signals &Signals::debug(char c) {
    _debug = c;
    return *this;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
