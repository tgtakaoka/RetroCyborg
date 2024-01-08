#include "signals_i8085.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_i8085.h"
#include "string_util.h"

namespace debugger {
namespace i8085 {

void Signals::getAddress() {
    s() = busRead(S);
    iom() = digitalReadFast(PIN_IOM);
    addr = busRead(AD) | busRead(AH);
}

void Signals::getDirection() {
    //    s() = busRead(S);
    rd() = digitalReadFast(PIN_RD);
    wr() = digitalReadFast(PIN_WR);
    inta() = digitalReadFast(PIN_INTA);
}

void Signals::getData() {
    data = busRead(AD);
}

void Signals::print() const {
    // clang-format off
    static char buffer[] = {
        ' ',                       // IO/#M or HALT=0
        'R',                       // #RD/#WR/#INTA=1
        ' ', 'A', '=', 0, 0, 0, 0, // addr=5
        ' ', 'D', '=', 0, 0,       // data=12
        0,
    };
    // clang-format on
    if (memory()) {
        buffer[0] = 'M';
        buffer[1] = fetch() ? 'F' : (read() ? 'R' : (write() ? 'W' : ' '));
    } else {
        buffer[0] = 'I';
        buffer[1] = vector() ? 'A' : (read() ? 'R' : (write() ? 'W' : ' '));
    }
    outHex16(buffer + 5, addr);
    outHex8(buffer + 12, data);
    cli.println(buffer);
}

}  // namespace i8085
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
