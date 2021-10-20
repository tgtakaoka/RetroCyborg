#ifndef __PINS_H__
#define __PINS_H__

#include <stdint.h>

#include "config.h"
#include "signals.h"

class Pins {
public:
    void begin();
    void loop();
    bool isRunning() const { return _freeRunning; }

    void print();
    void reset(bool show = false);
    void step(bool show = false);
    void halt(bool show = false);
    void run();

    void execInst(const uint8_t *inst, uint8_t len);
    uint8_t captureWrites(const uint8_t *inst, uint8_t len, uint16_t *addr,
            uint8_t *buf, uint8_t max);

    static constexpr uint16_t ioBaseAddress() { return IO_BASE_ADDR; }

    void assertIrq(const uint8_t mask);
    void negateIrq(const uint8_t mask = 0xff);
    uint8_t irqSignals(const uint8_t mask = 0xff) const { return _irq & mask; }
    static uint8_t getIrqMask(uint16_t addr) {
        return 1 << (addr - ioBaseAddress());
    }

private:
    bool _freeRunning;
    uint8_t _irq;

    Signals &cycle();
    uint8_t execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
            uint8_t *buf, uint8_t max);
};

extern Pins Pins;

#endif /* __PINS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
