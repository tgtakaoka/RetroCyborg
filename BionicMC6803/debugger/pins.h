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
    void idle();

    void execInst(const uint8_t *inst, uint8_t len);
    uint8_t captureWrites(const uint8_t *inst, uint8_t len, uint16_t *addr,
            uint8_t *buf, uint8_t max);

    static constexpr uint16_t ioBaseAddress() { return IO_BASE_ADDR; }

    uint8_t allocateIrq();
    void assertIrq(const uint8_t irq);
    void negateIrq(const uint8_t irq);

    enum SerialDevice : uint8_t {
        DEV_SCI = 0,   // MC6803/HD6303 SCI
        DEV_ACIA = 1,  // MC6850 ACIA
    };
    SerialDevice getIoDevice(uint16_t &baseAddr);
    void setIoDevice(SerialDevice device, uint16_t addr);

private:
    bool _freeRunning;
    uint8_t _irq;
    SerialDevice _ioDevice;

    Signals &cycle();
    uint8_t execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
            uint8_t *buf, uint8_t max);
    void suspend(bool show = false);
};

extern Pins Pins;

#endif /* __PINS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
