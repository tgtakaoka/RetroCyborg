#include "devs_f3850.h"
#include <string.h>
#include "debugger.h"
#include "i8251.h"

namespace debugger {
namespace f3850 {

struct DevsF3850 Devs;

void DevsF3850::reset() {
    USART.reset();
    USART.setBaseAddr(USART_BASE);
}

void DevsF3850::begin() {
    enableDevice(USART);
}

void DevsF3850::loop() {
    USART.loop();
}

bool DevsF3850::isSelected(uint32_t addr) const {
    return USART.isSelected(addr);
}

uint16_t DevsF3850::read(uint32_t addr) const {
    return USART.read(addr);
}

void DevsF3850::write(uint32_t addr, uint16_t data) const {
    USART.write(addr, data);
}

uint16_t DevsF3850::vector() const {
    return USART.vector();
}

Device &DevsF3850::parseDevice(const char *name) const {
    if (strcasecmp(name, USART.name()) == 0)
        return USART;
    return Devs::nullDevice();
}

void DevsF3850::enableDevice(Device &dev) {
    USART.enable(&dev == &USART);
}

void DevsF3850::printDevices() const {
    printDevice(USART);
}

}  // namespace f3850
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
