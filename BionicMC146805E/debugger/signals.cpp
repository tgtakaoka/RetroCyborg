#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli cli;

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
}

void Signals::disassembleCycles() {
    uint16_t lasti = 0;
    uint16_t nexti = 0;
    for (auto i = 0; i < _cycles; i++) {
        const auto pos = (i + _get) % MAX_CYCLES;
        const Signals &signals = _signals[pos];
        if (signals.fetchInsn()) {
            nexti = Regs.disassemble(lasti = signals.addr, 1);
        } else if (signals.rw == LOW) {
            cli.printHex(signals.addr, 4);
            cli.print(F(": W "));
            cli.printlnHex(signals.data, 2);
        } else {
            if (signals.addr < lasti || signals.addr >= nexti) {
                cli.printHex(signals.addr, 4);
                cli.print(F(": R "));
                cli.printlnHex(signals.data, 2);
            }
        }
        Pins.idle();
    }
}

Signals &Signals::currCycle() {
    return _signals[_put];
}

void Signals::resetCycles() {
    _cycles = 0;
    _signals[_get = _put].clear();
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
