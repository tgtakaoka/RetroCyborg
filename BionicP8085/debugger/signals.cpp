#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli cli;

static constexpr bool debug_cycles = false;

uint8_t Signals::_put = 0;
uint8_t Signals::_get = 0;
uint8_t Signals::_cycles = 0;
Signals Signals::_signals[MAX_CYCLES];

void Signals::clear() {
    // fields including _debug will be written in Pins::cycle().
    _inject = _capture = false;
}

void Signals::printCycles() {
    for (auto i = 0; i < _cycles; i++) {
        _signals[(i + _get) % MAX_CYCLES].print();
        Pins.idle();
    }
    _cycles = 0;
}

void Signals::disassembleCycles() {
    for (auto i = 0; i < _cycles; ) {
        const auto x = (i + _get) % MAX_CYCLES;
        const auto &signals = _signals[x];
        if (signals.fetchInsn()) {
            const auto next = Regs.disassemble(signals.addr, 1);
            i += next - signals.addr;
        } else {
            signals.print();
            ++i;
        }
        Pins.idle();
    }
}

Signals &Signals::currCycle() {
    return _signals[_put];
}

void Signals::resetCycles() {
    _cycles = 0;
    _get = _put;
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
    _signals[_put].clear();
}

void Signals::getAddress() {
    s = (Status)busRead(S);
    iom = digitalReadFast(PIN_IOM);
    addr = busRead(AD) | busRead(AH);
}

void Signals::getDirection() {
    s = (Status)busRead(S);
    iom = digitalReadFast(PIN_IOM);
    rd = digitalReadFast(PIN_RD);
    wr = digitalReadFast(PIN_WR);
    inta = digitalReadFast(PIN_INTA);
}

void Signals::getData() {
    data = busRead(AD);
}

Signals &Signals::inject(uint8_t val) {
    _inject = true;
    data = val;
    return *this;
}

void Signals::capture() {
    _capture = true;
}

void Signals::print() const {
    // clang-format off
    static char buffer[] = {
        ' ', ' ',                  // _debug=0
        ' ',                       // IO/#M or HALT=2
        'R',                       // #RD/#WR/#INTA=3
        ' ', 'A', '=', 0, 0, 0, 0, // addr=7
        ' ', 'D', '=', 0, 0,       // data=14
        0,
    };
    // clang-format off
    buffer[0] = _debug;
    if (iom) {
        buffer[2] = 'I';
        buffer[3] = (rd == LOW ? 'R' : (wr == LOW ? 'W' : (inta == LOW ? 'A' : ' ')));
    } else {
        buffer[2] = 'M';
        buffer[3] = (rd == LOW
                  ? (s == S_FETCH ? 'F' : 'R')
                  : (wr == LOW ? 'W' : ' '));
    }
    outHex16(buffer + 7, addr);
    outHex8(buffer + 14, data);
    cli.println(buffer);
}

Signals &Signals::debug(char c) {
    _debug = c;
    return *this;
}

Signals &Signals::setAddress(uint16_t address) {
    addr = address;
    return *this;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
