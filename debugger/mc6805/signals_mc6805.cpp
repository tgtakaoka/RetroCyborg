#include "signals_mc6805.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_mc6805.h"
#include "string_util.h"

namespace debugger {
namespace mc6805 {

void Signals::print() const {
    LOG(cli.printDec(pos(), -4));
    // clang-format off
    static char buffer[] = {
        'W',                       // rw=0
        ' ', 'A', '=', 0, 0, 0, 0, // addr=4
        ' ', 'D', '=', 0, 0,       // data=11
        0,
    };
    // clang-format on
    buffer[0] = fetch() ? 'L' : (write() ? 'W' : 'R');
    outHex16(buffer + 4, addr);
    outHex8(buffer + 11, data);
    cli.println(buffer);
}

}  // namespace mc6805
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
