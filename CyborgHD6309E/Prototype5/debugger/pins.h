/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
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
    void halt(bool show = false);
    void step(bool show = false);
    void run();
    void stopRunning();
    void execInst(const uint8_t *inst, uint8_t len, bool show = false);
    uint8_t captureReads(
            const uint8_t *inst, uint8_t len, uint8_t *capBuf, uint8_t max);
    uint8_t captureWrites(
            const uint8_t *inst, uint8_t len, uint8_t *capBuf, uint8_t max);

    void acknowledgeIoRequest();
    uint16_t ioRequestAddress() const;
    bool ioRequestWrite() const;
    uint8_t ioGetData();
    void ioSetData(uint8_t data);

    static constexpr uint16_t ioBaseAddress() { return IO_BASE_ADDR; }

    uint8_t allocateIrq();
    void assertIrq(const uint8_t irq);
    void negateIrq(const uint8_t irq);

    int sdCardChipSelectPin() const;

    enum SerialDevice : uint8_t {
        DEV_ACIA = 0,  // MC6850 ACIA
    };
    SerialDevice getIoDevice(uint16_t &baseAddr);
    void setIoDevice(SerialDevice device, uint16_t addr);

private:
    class Dbus {
    public:
        void begin();
        void set(uint8_t data);
        void output();
        void input();
        bool valid() const { return _valid; }
        void capture(bool enable);

    private:
        void setDbus(uint8_t dir, uint8_t data);
        uint8_t _data;
        bool _valid;
        bool _capture;
    };

    Signals &cycle(const Signals *prev);
    const Signals *unhalt();
    void setData(uint8_t data);
    uint8_t execute(const uint8_t *inst, uint8_t len, uint8_t *capBuf,
            uint8_t max, bool capWrite, bool capRead, bool show);

    Dbus _dbus;
    bool _freeRunning;
    bool _stopRunning;
    uint8_t _irq;
    SerialDevice _ioDevice;
};

extern Pins Pins;

#endif /* __PINS_H__ */
