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
}

bool Signals::fetchInsn() const {
    if (write())
        return false;
    const uint8_t pos = this - _signals;
    const uint8_t cycles = (pos + MAX_CYCLES - _get) % MAX_CYCLES;
    if (_cycles != MAX_CYCLES && cycles < 4)
        return false;
    const Signals &c0 = _signals[(pos + MAX_CYCLES - 4) % MAX_CYCLES];
    const Signals &c1 = _signals[(pos + MAX_CYCLES - 3) % MAX_CYCLES];
    const Signals &c2 = _signals[(pos + MAX_CYCLES - 2) % MAX_CYCLES];
    const Signals &c3 = _signals[(pos + MAX_CYCLES - 1) % MAX_CYCLES];
    if (addr == c2.addr + 1 && c2.addr == c1.addr + 1) {
        if (c1.read() && c2.read()) {
            // 8 bit operation
            if (debug_cycles)
                cli.println(F("8 bit operation"));
            return true;
        }
    }
    if (addr == c1.addr + 1 && c1.addr == c0.addr + 1) {
        if (c0.read() && c1.read()) {
            if (c3.addr == c2.addr + 1 && c2.read() == c3.read()) {
                // 16 bit operation
                if (debug_cycles)
                    cli.println(F("16 bit operation"));
                return true;
            }
            if (c3.addr == c2.addr && c2.read() && c3.write()) {
                // read-modify-write operation
                if (debug_cycles)
                    cli.println(F("read-modify-write operation"));
                return true;
            }
        }
    }
    if (c2.read() && (c2.data & ~0x1B) == 0x64 && c3.addr == c2.addr + 1 &&
            c3.read()) {
        const auto target = c3.addr + static_cast<int8_t>(c3.data);
        if (addr == target + 1) {
            // branch operation
            if (debug_cycles)
                cli.println(F("branch operation"));
            return true;
        }
    }
    if (c1.read() && c1.data == 0x24 && c2.addr == c1.addr + 1 &&
            c3.addr == c1.addr + 2 && c2.read() && c3.read()) {
        const auto target = (static_cast<uint16_t>(c3.data) << 8) | c2.data;
        if (addr == target + 1) {
            // jump operation
            if (debug_cycles)
                cli.println(F("jump operation"));
            return true;
        }
    }

    return false;
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

bool Signals::getDirection() {
    rds = digitalReadFast(PIN_RDS);
    wds = digitalReadFast(PIN_WDS);
    return rds == LOW || wds == LOW;
}

void Signals::getAddr() {
    addr = busRead(AL)
#if defined(AM_vp)
           | busRead(AM)
#endif
           | busRead(AH);
}

void Signals::getData() {
    data = busRead(D);
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
        'R',                       // RD/WR=2
        ' ', 'A', '=', 0, 0, 0, 0, // addr=6
        ' ', 'D', '=', 0, 0,       // data=13
        0,
    };
    // clang-format off
    buffer[0] = _debug;
    buffer[2] = (rds == LOW) ? 'R' : 'W';
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
