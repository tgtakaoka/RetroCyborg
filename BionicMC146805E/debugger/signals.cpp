#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

uint8_t Signals::_cycles;
Signals Signals::_signals[MAX_CYCLES + 1];

void Signals::clear() {
    // fields including _debug will be written in Pins::cycle().
    _inject = _capture = false;
}

void Signals::printCycles() {
    for (uint8_t i = 0; i < _cycles; i++) {
        _signals[i].print();
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
    if (_cycles < MAX_CYCLES)
        _cycles++;
    _signals[_cycles].clear();
}

extern uint8_t RAM[0x10000];

void Signals::getAddr1() {
    addr = busRead(AH);
}

void Signals::getAddr2() {
    addr |= busRead(B);
}

void Signals::getDirection() {
    rw = digitalReadFast(PIN_RW);
}

void Signals::getLoadInstruction() {
    li = digitalReadFast(PIN_LI);
}

void Signals::getData() {
    data = busRead(B);
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
        ' ',                       // _debug=0
        ' ', 'L',                  // li=2
        'W',                       // rw=3
        ' ', 'A', '=', 0, 0, 0, 0, // addr=7
        ' ', 'D', '=', 0, 0,       // data=14
        0,
    };
    // clang-format on
    buffer[0] = _debug;
    buffer[2] = (li == LOW) ? ' ' : 'L';
    buffer[3] = (rw == LOW) ? 'W' : 'R';
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
