#include "signals_cdp1802.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_cdp1802.h"
#include "string_util.h"

namespace debugger {
namespace cdp1802 {

void Signals::getStatus() {
    auto s = digitalReadFast(PIN_SC0);
    if (digitalReadFast(PIN_SC1))
        s += 2;
    sc() = s;
}

void Signals::getAddr1() {
    addr = static_cast<uint16_t>(busRead(MA)) << 8;
}

void Signals::getAddr2() {
    addr |= busRead(MA);
}

void Signals::getDirection() {
    mrd() = digitalReadFast(PIN_MRD);
    mwr() = digitalReadFast(PIN_MWR);
}

void Signals::getData() {
    data = busRead(DBUS);
}

void Signals::print() const {
    static constexpr char SC[] = { 'F', ' ', 'D', 'I' };
    // clang-format off
    static char buffer[] = {
        'L',                       // sc=0
        'W',                       // rds/wds=1
        ' ', 'A', '=', 0, 0, 0, 0, // addr=5
        ' ', 'D', '=', 0, 0,       // data=12
        0,
    };
    // clang-format on
    buffer[0] = SC[sc()];
    buffer[1] = read() ? 'R' : (write() ? 'W' : '-');
    outHex16(buffer + 5, addr);
    outHex8(buffer + 12, data);
    cli.println(buffer);
}

}  // namespace cdp1802
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
