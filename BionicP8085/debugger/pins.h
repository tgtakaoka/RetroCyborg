#ifndef __PINS_H__
#define __PINS_H__

#include <stdint.h>

#include "config.h"
#include "signals.h"

enum IntrName : uint8_t {
    INTR_NONE = 0,
    INTR_RST1 = 0x08,   // RST 1: 0008H
    INTR_RST2 = 0x10,   // RST 2: 0010H
    INTR_RST3 = 0x18,   // RST 3: 0018H
    INTR_RST4 = 0x20,   // RST 4: 0020H
    INTR_RST5 = 0x28,   // RST 5: 0028H
    INTR_RST6 = 0x30,   // RST 6: 0030H
    INTR_RST7 = 0x38,   // RST 7: 0038H
    INTR_RST55 = 0x2C,  // RST 5.5: 002CH
    INTR_RST65 = 0x34,  // RST 6.5: 0034H
    INTR_RST75 = 0x3C,  // RST 7.5: 003CH
    INTR_TRAP = 0x24,   // TRAP: 0024H
};

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

    void assertIntr(IntrName intr);
    void negateIntr(IntrName intr);

    enum Device : uint8_t {
        NONE = 0,
        USART = 1,  // i8251 UART
        SIO = 2,    // i8085 SOD/SID software UART
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
    uint8_t _inta;

    friend class Regs;
    void clk_hi() const;
    void clk_lo() const;
    void clk_cycle() const;
    Signals &cycleT1();
    Signals &cycleT2();
    Signals &cycleT2Ready();
    Signals &cycleT2Wait(uint16_t pc);
    Signals &cycleT3(Signals &signals);
    uint8_t execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
            uint8_t *buf, uint8_t max);

    Device _serialDevice;

    void setDeviceBase(Device dev, bool hasValue, uint16_t base);
    Device getSerialDevice(uint16_t &baseAddr) const;
    void setSerialDevice(Device dev, uint16_t addr);

    static uint8_t intrToInta(IntrName intr);
};

extern Pins Pins;

#endif /* __PINS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
