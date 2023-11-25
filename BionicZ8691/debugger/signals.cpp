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
    _inject = _capture = _fetch = false;
}

Signals &Signals::fetch(bool fetch) {
    _fetch = fetch;
    return *this;
}

void Signals::printCycles() {
    const auto n = _cycles;
    const auto g = _get;
    for (auto i = 0; i < n; i++) {
        const auto x = (i + g) % MAX_CYCLES;
        _signals[x].print();
        if (i % 4 == 3)
            Pins.idle();
    }
}

void Signals::disassembleCycles() {
    const auto n = _cycles;
    const auto g = _get;
    auto idle = 4;
    for (auto i = 0; i < n;) {
        const auto x = (i + g) % MAX_CYCLES;
        const auto &signals = _signals[x];
        const auto insn = signals.data;
        const auto len = Regs::insnLen(insn);
        if (signals.fetch() && len) {
            Regs.disassemble(signals.addr, 1);
            i += len;
            const auto cycles = Regs::busCycles(insn) - len;
            for (auto c = 0; c < cycles; c++) {
                const auto &s = _signals[(i + g + c) % MAX_CYCLES];
                s.print();
            }
            i += cycles;
        } else {
            signals.print();
            i++;
        }
        if (i >= idle) {
            Pins.idle();
            idle += 4;
        }
    }
}

Signals &Signals::currCycle() {
    return _signals[_put];
}

void Signals::resetCycles() {
    _cycles = 0;
    _signals[_get = _put].clear();
}

void Signals::resetTo(const Signals &signals) {
    _cycles = 0;
    _put = &signals - &_signals[0];
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

void Signals::getAddr() {
    addr = busRead(ADDR);
    rw = digitalReadFast(PIN_RW);
#if Z8_DATA_MEMORY == 1
    dm = digitalReadFast(PIN_DM);
#endif
}

void Signals::getData() {
    data = busRead(DATA);
}

void Signals::outData() {
    busWrite(DATA, data);
    busMode(DATA, OUTPUT);
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
#if Z8_DATA_MEMORY == 1
        ' ',                       // P/D=2
#endif
        'R',                       // R/W=3
        ' ', 'A', '=', 0, 0, 0, 0, // addr=7
        ' ', 'D', '=', 0, 0,       // data=14
        0,
    };
    // clang-format off
    buffer[0] = _debug;
#if Z8_DATA_MEMORY == 1
    buffer[2] = dm ? 'P' : 'D';
#endif
    buffer[2 + Z8_DATA_MEMORY] = read() ? 'R' : 'W';
    outHex16(buffer + 6 + Z8_DATA_MEMORY, addr);
    outHex8(buffer + 13 + Z8_DATA_MEMORY, data);
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
