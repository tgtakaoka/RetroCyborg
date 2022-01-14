
#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli &cli;

uint8_t Signals::_cycles;
Signals Signals::_signals[MAX_CYCLES + 1];
Signals::MpuType Signals::_type = Signals::MpuType::MOS6502;

void Signals::clear() {
    // fields including _debug will be written in Pins::cycle().
    _inject = _capture = false;
}

void Signals::printCycles(const Signals *end) {
    if (end == nullptr)
        end = &currCycle();
    for (const auto *signals = _signals; signals < end; signals++) {
        signals->print();
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

void Signals::getAddr() {
    rw = digitalReadFast(PIN_RW);
    addr = busRead(AL)
#if defined(AM_vp)
           | busRead(AM)
#endif
           | busRead(AH);
    if (_type == MpuType::MOS6502) {
        _vector = rw != LOW && addr >= 0xFFFA;
        _lock = false;
        _waiting = false;
    } else {
        _vector = digitalReadFast(PIN_VP) == LOW;
        _lock = digitalReadFast(PIN_ML) == LOW;
        _waiting = digitalReadFast(PIN_RDY) == LOW;
    }
    if (_type == MpuType::W65C816S) {
        const auto vpa = digitalReadFast(W65C816S_VPA);
        const auto vda = digitalReadFast(W65C816S_VDA);
        _insn = vpa != LOW && vda != LOW;
    } else {
        _insn = digitalReadFast(PIN_SYNC) != LOW;
    }
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
        ' ',                       // _debug=0
        'R',                       // rdy=1
        'V',                       // vp/sync/ml=2
        'W',                       // rw=3
        ' ', 'A', '=', 0, 0, 0, 0, // addr=7
        ' ', 'D', '=', 0, 0,       // data=14
        0,
    };
    // clang-format on
    buffer[0] = _debug;
    buffer[1] = waiting() ? 'R' : ' ';
    buffer[2] =
            fetchInsn() ? 'S' : (fetchVector() ? 'V' : (busLock() ? 'L' : ' '));
    buffer[3] = (rw == LOW) ? 'W' : 'R';
    outHex16(buffer + 7, addr);
    outHex8(buffer + 14, data);
    cli.println(buffer);
}

Signals &Signals::debug(char c) {
    _debug = c;
    return *this;
}

void Signals::checkHardwareType() {
    digitalWriteFast(PIN_PHI0, LOW);
    delayNanoseconds(500);
    digitalWriteFast(PIN_PHI0, HIGH);
    delayNanoseconds(500);

    // See Pins::begin() for details.
    if (digitalReadFast(PIN_PHI1O) == LOW) {
        // PIN_PHI1O is inverting of PHI0
        if (digitalReadFast(PIN_VP) == LOW) {
            // PIN_VP keeps LOW, means Vss of MOS6502, G65SC02, R65C02.
            _type = MpuType::MOS6502;
        } else {
            // PIN_VP goes HIGH, means #VP of W65C02S.
            _type = MpuType::W65C02S;
        }
        digitalWriteFast(PIN_SO, HIGH);
        pinMode(PIN_SO, OUTPUT);
    } else {
        // PIN_PHI1O keeps HIGH, means pulled-up #ABORT of W65C816S.
        _type = MpuType::W65C816S;
        digitalWriteFast(W65C816S_ABORT, HIGH);
        pinMode(W65C816S_ABORT, OUTPUT);
    }

    digitalWriteFast(PIN_PHI0, LOW);
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
