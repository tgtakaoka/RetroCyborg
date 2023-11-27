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

void Signals::printCycles() {
    for (auto i = 0; i < _cycles; i++) {
        _signals[(i + _get) % MAX_CYCLES].print();
        Pins.idle();
    }
    _cycles = 0;
}

void Signals::disassembleCycles() {
    const auto n = _cycles;
    const auto g = _get;
    for (auto i = 0; i < n;) {
        const auto x = (i + g) % MAX_CYCLES;
        const auto &signals = _signals[x];
        const auto insn = signals.data;
        const auto len = Regs::insnLen(insn);
        if (signals.fetch() && len) {
            Regs.disassemble(signals.addr, 1);
            auto cycles = Regs::busCycles(insn);
            if (Regs::hasIndirect(insn)) {
                const auto y = (i + 1 + g) % MAX_CYCLES;
                if (_signals[y].data & 0x80)
                    cycles += 2;
            }
            i += len;
            for (auto c = 0; c < cycles; c++) {
                const auto &s = _signals[(i + g + c) % MAX_CYCLES];
                s.print();
            }
            i += cycles;
        } else {
            signals.print();
            i++;
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
        _get = _put;
    }
    _signals[_put].clear();
}

Signals &Signals::getAddr() {
    addr = busRead(ADRL) | busRead(ADRM) | busRead(ADRH);
    _mio = digitalReadFast(PIN_MIO);
    _rw = digitalReadFast(PIN_RW);
    return *this;
}

bool Signals::ioAccess() const {
    return _mio == 0;
}

void Signals::getData() {
    data = busRead(DBUS);
}

void Signals::outData() {
    busWrite(DBUS, data);
    busMode(DBUS, OUTPUT);
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
        'M',                       // #M/IO=2
        'R',                       // #R/W=3
        ' ', 'A', '=', 0, 0, 0, 0, // addr=7
        ' ', 'D', '=', 0, 0,       // data=14
        0,
    };
    // clang-format off
    buffer[0] = _debug;
    buffer[2] = _mio ? 'M' : 'I';
    buffer[3] = _rw ? 'W' : 'R';
    if (_mio) {
        outHex16(buffer + 7, addr);
    } else {
        if (addr & 0x2000) {
            buffer[7] = 'E';
            buffer[8] = ':';
            outHex8(buffer + 9, addr);
        } else {
            buffer[7] = 'N';
            buffer[8] = 'E';
            buffer[9] = ':';
            buffer[10] = (addr & 0x4000) ? 'D' : 'C';
        }
    }
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
