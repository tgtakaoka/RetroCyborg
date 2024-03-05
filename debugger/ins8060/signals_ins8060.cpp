#include "signals_ins8060.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_ins8060.h"
#include "string_util.h"

namespace debugger {
namespace ins8060 {

void Signals::getAddr() {
    const auto db = busRead(DB);
    flags() = db;
    addr = (static_cast<uint16_t>(db & 0xF) << 12) | busRead(ADM) |
           busRead(ADL);
}

void Signals::getData() {
    data = busRead(DB);
}

void Signals::print() const {
    LOG(cli.printDec(pos(), -4));
    // clang-format off
    static char buffer[] = {
        ' ',                       // I/D/H=0
        'R',                       // R/W=1
        ' ', 'A', '=', 0, 0, 0, 0, // addr=5
        ' ', 'D', '=', 0, 0,       // data=12
        0,
    };
    // clang-format on
    buffer[0] = fetch() ? 'I' : (delay() ? 'D' : (halt() ? 'H' : ' '));
    buffer[1] = read() ? 'R' : 'W';
    outHex16(buffer + 5, addr);
    outHex8(buffer + 12, data);
    cli.println(buffer);
}

}  // namespace ins8060
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
