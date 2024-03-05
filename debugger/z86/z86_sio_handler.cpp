#include "z86_sio_handler.h"
#include "pins_z86.h"
#include "regs_z86.h"

namespace debugger {
namespace z86 {

Z86SioHandler::Z86SioHandler() : SerialHandler(PIN_RXD, PIN_TXD) {}

const char *Z86SioHandler::name() const {
    return "SIO";
}

const char *Z86SioHandler::description() const {
    return Regs.cpuName();
}

uint32_t Z86SioHandler::baseAddr() const {
    return 0xF0;
}

void Z86SioHandler::resetHandler() {
    // Z8 SIO: assuming XTAL is 14.7546MHz
    // bit rate = 1475600 / (2 x 4 x p x t x 16)
    _pre_divider = 2 * 4 * 1 * 16;  // p=1
    _divider = 12;                  // t=12
}

}  // namespace z86
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
