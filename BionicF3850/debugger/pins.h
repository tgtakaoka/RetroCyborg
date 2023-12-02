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
    uint8_t captureWrites(
            const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max);

    void assertIntreq();
    void negateIntreq();

    enum Device : uint8_t {
        NONE = 0,
        USART = 1,  // i8251
    };
    Device parseDevice(const char *name) const;
    void getDeviceName(Device dev, char *name) const;
    void setDeviceBase(Device dev, uint16_t base) {
        setDeviceBase(dev, true, base);
    }
    void setDeviceBase(Device dev) { setDeviceBase(dev, false, 0); }
    void printDevices() const;

    void setRomArea(uint16_t begin, uint16_t end);
    void printRomArea() const;

    bool setBreakPoint(uint16_t addr);
    bool clearBreakPoint(uint8_t index);
    void printBreakPoints() const;

private:
    bool _freeRunning;

    Signals &cycle();
    bool rawStep();
    uint8_t execute(
            const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max);

    Device _serialDevice;
    void setDeviceBase(Device dev, bool hasValue, uint16_t base);
    Device getSerialDevice(uint16_t &baseAddr) const;
    void setSerialDevice(Device dev, uint16_t addr);

    uint16_t _rom_begin;
    uint16_t _rom_end;

    uint8_t _breakNum;
    uint16_t _breakPoints[4];
    uint8_t _breakInsns[4];
    void saveBreakInsns();
    void restoreBreakInsns();
    bool isBreakPoint(uint16_t addr) const;
};

extern Pins Pins;

#endif /* __PINS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
