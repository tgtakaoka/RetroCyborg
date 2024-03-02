#include "i8085_sio_handler.h"
#include "pins_i8085.h"
#include "regs_i8085.h"

namespace debugger {
namespace i8085 {

I8085SioHandler::I8085SioHandler() : SerialHandler(PIN_SID, PIN_SOD) {}

const char *I8085SioHandler::name() const {
    return "SIO";
}

const char *I8085SioHandler::description() const {
    return Regs.cpuName();
}

uint32_t I8085SioHandler::baseAddr() const {
    return 0x00;
}

void I8085SioHandler::resetHandler() {
    // I8085 bitbang speed: assuming CLK is 3MHz
    _pre_divider = 24;
    _divider = 13;
}

}  // namespace i8085
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
