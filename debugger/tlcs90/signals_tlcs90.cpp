#include "signals_tlcs90.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_tlcs90.h"
#include "string_util.h"

namespace debugger {
namespace tlcs90 {

void Signals::getAddrHigh() {
    addr = busRead(ADM) | busRead(ADH);
}

void Signals::getAddrLow() {
    addr |= busRead(ADL);
}

void Signals::getDirection() {
    rd() = digitalReadFast(PIN_RD);
    wr() = digitalReadFast(PIN_WR);
}

Signals *Signals::current() {
    auto s = put();
    s->getAddrHigh();
    s->getAddrLow();
    s->getDirection();
    return s;
}

void Signals::getData() {
    data = busRead(DB);
}

void Signals::print() const {
    LOG_MATCH(cli.printDec(pos(), -4));
    static char buffer[] = {
            ' ',                        // #RD/#WR=0
            ' ', 'A', '=', 0, 0, 0, 0,  // addr=4
            ' ', 'D', '=', 0, 0,        // data=11
            0,                          // eos
    };
    buffer[0] = read() ? 'R' : (write() ? 'W' : ' ');
    outHex16(buffer + 4, addr);
    outHex8(buffer + 11, data);
    cli.println(buffer);
}

}  // namespace tlcs90
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
