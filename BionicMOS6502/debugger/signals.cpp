#include "signals.h"

#include <libcli.h>

#include "digital_fast.h"
#include "pins.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli &cli;

uint8_t Signals::_put = 0;
uint8_t Signals::_get = 0;
uint8_t Signals::_cycles = 0;
Signals Signals::_signals[MAX_CYCLES];
Signals::MpuType Signals::_type = Signals::MpuType::MOS6502;

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
        } else if (signals.rw == 0) {
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
    } else {
        _vector = digitalReadFast(PIN_VP) == LOW;
        _lock = digitalReadFast(PIN_ML) == LOW;
    }
    if (_type == MpuType::W65C816S) {
        addr |= static_cast<uint32_t>(busRead(D)) << 16;
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
    char *buffer;
    if (native65816()) {
        // clang-format off
        static char b65816[] = {
            ' ',                             // _debug=0
            'R',                             // rdy=1
            'V',                             // vp/sync/ml=2
            'W',                             // rw=3
            ' ', 'A', '=', 0, 0, 0, 0, 0, 0, // addr=7
            ' ', 'D', '=', 0, 0,             // data=16
            0,
        };
        // clang-format on
        buffer = b65816;
        outHex24(buffer + 7, addr);
        outHex8(buffer + 16, data);
    } else {
        // clang-format off
        static char b6502[] = {
            ' ', ' ',                  // _debug=0
            'V',                       // vp/sync/ml=2
            'W',                       // rw=3
            ' ', 'A', '=', 0, 0, 0, 0, // addr=7
            ' ', 'D', '=', 0, 0,       // data=14
            0,
        };
        // clang-format on
        buffer = b6502;
        outHex16(buffer + 7, addr);
        outHex8(buffer + 14, data);
    }
    buffer[0] = _debug;
    buffer[2] =
            fetchInsn() ? 'S' : (fetchVector() ? 'V' : (busLock() ? 'L' : ' '));
    buffer[3] = (rw == LOW) ? 'W' : 'R';
    cli.println(buffer);
}

Signals &Signals::debug(char c) {
    _debug = c;
    return *this;
}

bool Signals::native65816() {
    return _type == MpuType::W65C816S && digitalReadFast(PIN_E) == LOW;
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
