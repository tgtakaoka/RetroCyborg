#include "signals_z86.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_z86.h"
#include "string_util.h"

namespace debugger {
namespace z86 {

void Signals::getAddr() {
    addr = busRead(ADDR);
    rw() = digitalReadFast(PIN_RW);
#if Z8_DATA_MEMORY == 1
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
#if Z8_DATA_MEMORY == 1
        ' ',                       // P/D=0
#endif
        'R',                       // R/W=0
        ' ', 'A', '=', 0, 0, 0, 0, // addr=4
        ' ', 'D', '=', 0, 0,       // data=11
        0,
    };
    // clang-format on
#if Z8_DATA_MEMORY == 1
    buffer[0] = dm() ? 'P' : 'D';
#endif
    buffer[0 + Z8_DATA_MEMORY] = read() ? 'R' : 'W';
    outHex16(buffer + 4 + Z8_DATA_MEMORY, addr);
    outHex8(buffer + 11 + Z8_DATA_MEMORY, data);
    cli.println(buffer);
}

}  // namespace z86
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
