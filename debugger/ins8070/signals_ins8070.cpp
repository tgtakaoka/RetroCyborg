#include "signals_ins8070.h"
#include "debugger.h"
#include "digital_bus.h"
#include "inst_ins8070.h"
#include "pins_ins8070.h"
#include "string_util.h"

namespace debugger {
namespace ins8070 {

bool Signals::fetch() const {
    // check at least 5 bus cycles.
    // this may not work for SSM instruction.
    if (write() || _cycles < 6)
        return false;
    InstIns8070 inst;
    const auto end = next();
    // needs at least 2 valid bus cycles.
    for (auto i = 2; i < 6; ++i) {
        if (inst.match(prev(i), end))
            return inst.matchedCycles() == i;
    }
    return false;
}

bool Signals::getDirection() {
    rds() = digitalReadFast(PIN_RDS);
    wds() = digitalReadFast(PIN_WDS);
    return read() || write();
}

void Signals::getAddr() {
    addr = busRead(AL) | busRead(AM) | busRead(AH);
}

void Signals::getData() {
    data = busRead(D);
    markFetch(false);
}

void Signals::print() const {
    LOG_MATCH(cli.printDec(pos(), -4));
    static char buffer[] = {
            'R',                        // RD/WR=0
            ' ', 'A', '=', 0, 0, 0, 0,  // addr=4
            ' ', 'D', '=', 0, 0,        // data=11
            0,                          // eos
    };
    buffer[0] = read() ? 'R' : 'W';
    outHex16(buffer + 4, addr);
    outHex8(buffer + 11, data);
    cli.println(buffer);
}

}  // namespace ins8070
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
