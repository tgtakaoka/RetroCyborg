#include "signals_z88.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_z88.h"
#include "string_util.h"

namespace debugger {
namespace z88 {

void Signals::getAddr() {
    addr = busRead(ADDR);
    rw() = digitalReadFast(PIN_RW);
#if Z88_DATA_MEMORY == 1
    dm() = digitalReadFast(PIN_DM);
#endif
}

void Signals::getData() {
    data = busRead(DATA);
    fetch() = false;
}

void Signals::outData() {
    busWrite(DATA, data);
    busMode(DATA, OUTPUT);
}

void Signals::print() const {
    // clang-format off
    static char buffer[] = {
#if Z88_DATA_MEMORY == 1
        ' ',                       // P/D=0
#endif
        'R',                       // R/#W=0
        ' ', 'A', '=', 0, 0, 0, 0, // addr=4
        ' ', 'D', '=', 0, 0,       // data=11
        0,
    };
    // clang-format on
#if Z88_DATA_MEMORY == 1
    buffer[0] = dm() ? 'P' : 'D';
#endif
    buffer[0 + Z88_DATA_MEMORY] = read() ? 'R' : 'W';
    outHex16(buffer + 4 + Z88_DATA_MEMORY, addr);
    outHex8(buffer + 11 + Z88_DATA_MEMORY, data);
    cli.println(buffer);
}

}  // namespace z88
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=
