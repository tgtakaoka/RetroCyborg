#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"
#include "regs.h"

extern libcli::Cli &cli;

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
        const auto x = (i + _get) % MAX_CYCLES;
        const Signals &signals = _signals[x];
        if (signals.fetchInsn()) {
            nexti = Regs.disassemble(lasti = signals.addr, 1);
            if (signals.data == 0x68) {
                // double fetch instructions, skip next bus cycle.
                i++;
            }
        } else if (signals.mwr == LOW) {
            cli.printHex(signals.addr, 4);
            cli.print(F(": W "));
            cli.printlnHex(signals.data, 2);
        } else if (signals.mrd == LOW) {
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
