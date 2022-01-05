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

    uint8_t allocateIrq();
    void assertIrq(const uint8_t irq);
    void negateIrq(const uint8_t irq);

    enum Device : uint8_t {
        NONE = 0,
        ACIA = 1,  // MC6850 ACIA
    };
    Device parseDevice(const char *name) const;
    void getDeviceName(Device dev, char *name) const;
    void setDeviceBase(Device dev, uint16_t base) {
        setDeviceBase(dev, true, base);
    }
    void setDeviceBase(Device dev) { setDeviceBase(dev, false, 0); }
    void printDevices() const;

private:
    bool _freeRunning;
    uint8_t _irq;

    Signals &cycle();
    Signals &raw_cycle();
    uint8_t execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
            uint8_t *buf, uint8_t max);

    Device _serialDevice;

    void setDeviceBase(Device dev, bool hasValue, uint16_t base);
    Device getSerialDevice(uint16_t &baseAddr) const;
    void setSerialDevice(Device dev, uint16_t addr);
};

extern Pins Pins;

#endif /* __PINS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
