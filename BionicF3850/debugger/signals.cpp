#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "inst.h"
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
    _inject = _capture = false;
    _debug = ' ';
}

void Signals::printCycles() {
    const auto &get = _signals[_get];
    for (auto i = 0; i < _cycles; ++i) {
        get.next(i).print();
    }
    resetCycles();
}

bool matchAll(const Signals &begin, const Signals &end) {
    const auto cycles = begin.diff(end);
    const auto limit = cycles * 2 / 3;
    const Signals *prefetch = nullptr;
    LOG(cli.print(F("@@ matchAll: begin=")));
    LOG(begin.print());
    for (auto i = 0; i < limit;) {
        const auto &s = begin.next(i);
        LOG(cli.print(F("@@  i=")));
        LOG(cli.printlnDec(i));
        Inst inst;
        const auto &bus = prefetch ? *prefetch : s;
        if (bus.read() && inst.get(bus.addr)) {
            if (inst.match(s, end, prefetch)) {
                const auto len = inst.instLen - (prefetch ? 1 : 0);
                i += len + inst.busCycles;
                prefetch = &s.next(len + inst.prefetch);
                continue;
            }
            LOG(cli.print(F("@@ prefetch=")));
            LOG(cli.printlnHex(prefetch ? prefetch->addr : 0, 4));
            prefetch = nullptr;
            for (auto j = i; j < i + 4; ++j) {
                const auto &n = begin.next(j);
                if (n.read() && n.addr == bus.addr + 1 && inst.match(n, end, &bus)) {
                    const auto len = inst.instLen - 1;
                    i = j + len + inst.busCycles;
                    prefetch = &n.next(len + inst.prefetch);
                    break;
                }
            }
            if (prefetch) {
                LOG(cli.print(F("@@ next prefetch=")));
                LOG(cli.printlnHex(prefetch->addr, 4));
                continue;
            }
        }
        LOG(cli.print(F("@@ matchAll: FALSE i=")));
        LOG(cli.printlnDec(i));
        return false;
    }
    LOG(cli.println(F("@@ matchAll: MATCH")));
    return true;
}

const Signals &find_fetch(const Signals &begin, const Signals &end) {
    const auto cycles = begin.diff(end);
    const auto limit = cycles >= 10 ? 10 : cycles;
    for (auto i = 0; i < limit; ++i) {
        if (matchAll(begin.next(i), end))
            return begin.next(i);
    }
    return end;
}

void Signals::disassembleCycles(const Signals &end) {
    const auto &get = _signals[_get];
    const auto &begin = find_fetch(get, end);
    const auto num = get.diff(begin);
    for (auto i = 0; i < num; ++i) {
        get.next(i).print();
    }
    const auto cycles = begin.diff(end);
    const Signals *prefetch = nullptr;
    for (auto i = 0; i < cycles;) {
        const auto &s = begin.next(i);
        Inst inst;
        const auto &bus = prefetch ? *prefetch : s;
        if (bus.read() && inst.get(bus.addr)) {
            if (inst.match(s, end, prefetch)) {
                Memory.disassemble(bus.addr, 1);
                const auto len = inst.instLen - (prefetch ? 1 : 0);
                for (auto j = 0; j < inst.busCycles; ++j)
                    s.next(len + j).print();
                i += len + inst.busCycles;
                prefetch = &s.next(len + inst.prefetch);
                continue;
            }
            prefetch = nullptr;
            for (auto j = i; j < i + 4; ++j) {
                const auto &n = begin.next(j);
                if (n.read() && n.addr == bus.addr + 1 && inst.match(n, end, &bus)) {
                    Memory.disassemble(bus.addr, 1);
                    const auto len = inst.instLen - 1;
                    for (auto j = 0; j < inst.busCycles; ++j)
                        n.next(len + j).print();
                    i = j + len + inst.busCycles;
                    prefetch = &n.next(len + inst.prefetch);
                    break;
                }
            }
            if (prefetch)
                continue;
        }
        s.print();
        ++i;
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

Signals &Signals::getAddr() {
    addr = busRead(ADL) | busRead(ADM) | busRead(ADH);
    _rd = digitalReadFast(PIN_RD);
    _wr = digitalReadFast(PIN_WR);
    return *this;
}

void Signals::getData() {
    data = busRead(DB);
}

void Signals::outData() {
    busWrite(DB, data);
    busMode(DB, OUTPUT);
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
        ' ',                       // #RD/#WR=2
        ' ', 'A', '=', 0, 0, 0, 0, // addr=6
        ' ', 'D', '=', 0, 0,       // data=13
        0,
    };
    // clang-format off
    buffer[0] = _debug;
    buffer[2] = (_rd == 0) ? 'R' : (_wr == 0 ? 'W' : ' ');
    outHex16(buffer + 6, addr);
    outHex8(buffer + 13, data);
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
