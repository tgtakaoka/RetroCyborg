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
    _cycles++;
    _cycles %= MAX_CYCLES;
    _signals[_cycles].clear();
}

void Signals::getStatus() {
    uint8_t status = digitalReadFast(PIN_SC1) != LOW ? 2 : 0;
    if (digitalReadFast(PIN_SC0) != LOW)
        status += 1;
    sc = (Status) status;
}

void Signals::getAddr1() {
    addr = static_cast<uint16_t>(busRead(MA)) << 8;
    n = busRead(N);
}

void Signals::getAddr2() {
    addr |= busRead(MA);
    mrd = digitalReadFast(PIN_MRD);
}

void Signals::getDirection() {
    mwr = digitalReadFast(PIN_MWR);
}

void Signals::getData() {
    data = busRead(DBUS);
}

Signals &Signals::inject(uint8_t val) {
    Signals &curr = currCycle();
    curr._inject = true;
    curr.data = val;
    return curr;
}

void Signals::capture() {
    Signals &curr = currCycle();
    curr._capture = true;
}

void Signals::print() const {
    static const char sc_state[] = {'F', ' ', 'D', 'R', '?'};
    // clang-format off
    static char buffer[] = {
        ' ',                       // _debug=0
        ' ',
        'L',                       // sc=2
        'W',                       // rds/wds=3
        ' ', 'A', '=', 0, 0, 0, 0, // addr=7
        ' ', 'D', '=', 0, 0,       // data=14
        ' ', 'N', '=', 0,          // n=19
        0,
    };
    // clang-format on
    buffer[0] = _debug ? _debug : ' ';
    buffer[2] = sc_state[(uint8_t)sc];
    buffer[3] = (mrd == LOW) ? 'R' : (mwr == LOW ? 'W' : '-');
    outHex16(buffer + 7, addr);
    outHex8(buffer + 14, data);
    if (n) {
        buffer[16] = ' ';
        outHex4(buffer + 19, n);
    } else {
        buffer[16] = 0;
    }
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
