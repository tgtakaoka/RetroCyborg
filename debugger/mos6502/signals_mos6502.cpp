#include "signals_mos6502.h"
#include "debugger.h"
#include "digital_bus.h"
#include "inst_mos6502.h"
#include "pins_mos6502.h"
#include "string_util.h"

namespace debugger {
namespace mos6502 {

void Signals::getAddr() {
    rw() = digitalReadFast(PIN_RW);
    addr = busRead(AL) | busRead(AM) | busRead(AH);
    if (Pins.hardwareType() == HW_MOS6502) {
        fetchVector() = read() && addr >= InstMos6502::VECTOR_NMI;
        lock() = false;
    } else {
        fetchVector() = digitalReadFast(PIN_VP) == LOW;
        lock() = digitalReadFast(PIN_ML) == LOW;
    }
    if (Pins.hardwareType() == HW_W65C816) {
        addr |= static_cast<uint32_t>(busRead(D)) << 16;
        const auto vpa = digitalReadFast(W65C816_VPA);
        const auto vda = digitalReadFast(W65C816_VDA);
        sync() = (vpa != LOW) && (vda != LOW);
    } else {
        sync() = digitalReadFast(PIN_SYNC) != LOW;
    }
}

void Signals::getData() {
    data = busRead(D);
}

void Signals::print() const {
    char *buffer;
    if (Pins.native65816()) {
        // clang-format off
        static char b65816[] = {
            'V',                             // vp/sync/ml=0
            'W',                             // rw=1
            ' ', 'A', '=', 0, 0, 0, 0, 0, 0, // addr=5
            ' ', 'D', '=', 0, 0,             // data=14
            0,
        };
        // clang-format on
        buffer = b65816;
        outHex24(buffer + 5, addr);
        outHex8(buffer + 14, data);
    } else {
        // clang-format off
        static char b6502[] = {
            'V',                       // vp/sync/ml=0
            'W',                       // rw=1
            ' ', 'A', '=', 0, 0, 0, 0, // addr=5
            ' ', 'D', '=', 0, 0,       // data=12
            0,
        };
        // clang-format on
        buffer = b6502;
        outHex16(buffer + 5, addr);
        outHex8(buffer + 12, data);
    }
    buffer[0] = sync() ? 'S' : (vector() ? 'V' : (lock() ? 'L' : ' '));
    buffer[1] = write() ? 'W' : 'R';
    cli.println(buffer);
}

}  // namespace mos6502
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
