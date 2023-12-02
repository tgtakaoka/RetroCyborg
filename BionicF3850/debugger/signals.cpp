#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "regs.h"
#include "string_util.h"

#define LOG(e)
//#define LOG(e) e

extern libcli::Cli cli;

uint8_t Signals::_put = 0;
uint8_t Signals::_get = 0;
uint8_t Signals::_cycles = 0;
Signals Signals::_signals[MAX_CYCLES];

void Signals::clear() {
    _read = _write = _fetch = _io = false;
    _inject = _capture = false;
    _debug = ' ';
}

void Signals::printCycles() {
    const auto &get = _signals[_get];
    LOG(cli.print(F("@@ printCycles: cycles=")));
    LOG(cli.printDec(_cycles));
    LOG(cli.print(F(" get=")));
    LOG(cli.printlnDec(&get - _signals));
    for (auto i = 0; i < _cycles; ++i) {
        const auto &s = get.next(i);
        s.print();
    }
    resetCycles();
}

void Signals::disassembleCycles() {
    const auto &get = _signals[_get];
    LOG(cli.print(F("@@ disassembleCycles: cycles=")));
    LOG(cli.printDec(_cycles));
    LOG(cli.print(F(" get=")));
    LOG(cli.printlnDec(&get - _signals));
    for (auto i = 0; i < _cycles;) {
        const auto &s = get.next(i);
        const auto len = Regs::insnLen(s.data);
        if (s.fetch() && len) {
            Memory.disassemble(s.addr, 1);
            const auto cycles = Regs::busCycles(s.data);
            for (auto j = 0; j < cycles; ++j)
                s.next(len + j).print();
            i += len + cycles;
        } else {
            s.print();
            ++i;
        }
    }
    resetCycles();
}

Signals &Signals::currCycle() {
    return _signals[_put];
}

const Signals &Signals::prev(uint8_t backward) const {
    const auto pos = this - _signals;
    return _signals[(pos + MAX_CYCLES - backward) % MAX_CYCLES];
}

const Signals &Signals::next(uint8_t forward) const {
    const auto pos = this - _signals;
    return _signals[(pos + forward) % MAX_CYCLES];
}

uint8_t Signals::diff(const Signals &s) const {
    return (this < &s ? &s - this : (&s + MAX_CYCLES) - this) % MAX_CYCLES;
}

void Signals::resetCycles() {
    _cycles = 0;
    _signals[_get = _put].clear();
}

void Signals::nextCycle() {
    _put = (_put + 1) % MAX_CYCLES;
    if (_cycles < MAX_CYCLES) {
        _cycles++;
    } else {
        _get = (_put + 1) % MAX_CYCLES;
    }
    _signals[_put].clear();
}

Signals &Signals::getRomc() {
    romc = busRead(ROMC);
    return *this;
}

Signals &Signals::getData() {
    data = busRead(DB);
    return *this;
}

void Signals::outData() {
    busWrite(DB, data);
    busMode(DB, OUTPUT);
}

void Signals::markRead(uint16_t a) {
    addr = a;
    _read = true;
}

void Signals::markWrite(uint16_t a) {
    addr = a;
    _write = true;
}

void Signals::markIoRead(uint8_t a) {
    addr = a;
    _io = true;
    _read = true;
}

void Signals::markIoWrite(uint8_t a) {
    addr = a;
    _io = true;
    _write = true;
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
    LOG(cli.printDec(this - _signals, -3));
    // clang-format off
    static char buffer[] = {
        0, ' ',                    // _debug=0
        0, 0, ' ',                 // romc=2
        0, ' ',                    // read/write=5
        'A', '=', 0, 0, 0, 0, ' ', // addr=9
        'D', '=', 0, 0,            // data=16
        0,
    };
    // clang-format on
    buffer[0] = _debug;
    outHex8(buffer + 2, romc);
    if (_read || _write) {
        buffer[5] = _read ? 'R' : 'W';
        if (_io) {
            buffer[9] = 'I';
            buffer[10] = ' ';
            outHex8(buffer + 11, addr);
        } else {
            outHex16(buffer + 9, addr);
        }
    } else {
        buffer[5] = ' ';
        buffer[9] = '_';
        buffer[10] = '_';
        buffer[11] = '_';
        buffer[12] = '_';
    }
    outHex8(buffer + 16, data);
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
