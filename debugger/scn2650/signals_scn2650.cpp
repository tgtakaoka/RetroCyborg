#include "signals_scn2650.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_scn2650.h"
#include "string_util.h"

namespace debugger {
namespace scn2650 {

void Signals::getAddr() {
    addr = busRead(ADRL) | busRead(ADRM) | busRead(ADRH);
    rw() = digitalReadFast(PIN_RW);
    mio() = digitalReadFast(PIN_MIO);
    intack() = digitalReadFast(PIN_INTACK);
    fetchMark() = false;
}

void Signals::getData() {
    data = busRead(DBUS);
}

void Signals::outData() {
    busWrite(DBUS, data);
    busMode(DBUS, OUTPUT);
}

void Signals::print() const {
    // clang-format off
    static char buffer[] = {
        'M',                       // #M/IO,INTACK=0
        'R',                       // #R/W=1
        ' ', 'A', '=', 0, 0, 0, 0, // addr=5
        ' ', 'D', '=', 0, 0,       // data=12
        0,
    };
    // clang-format on
    buffer[0] = io() ? (intack() ? 'V' : 'I') : 'M';
    buffer[1] = write() ? 'W' : 'R';
    if (io()) {
        if (addr & 0x2000) {
            buffer[5] = 'E';
            buffer[6] = ':';
            outHex8(buffer + 7, addr);
        } else {
            buffer[5] = 'N';
            buffer[6] = 'E';
            buffer[7] = ':';
            buffer[8] = (addr & 0x4000) ? 'D' : 'C';
        }
    } else {
        outHex16(buffer + 5, addr);
    }
    outHex8(buffer + 12, data);
    cli.println(buffer);
}

}  // namespace scn2650
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
