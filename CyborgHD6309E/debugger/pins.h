/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __PINS_H__
#define __PINS_H__

#include <Arduino.h>
#include <stdint.h>

class Signals {
public:
    void get();
    void print() const;
    uint8_t dbus() const { return _dbus; }
    bool running() const { return (_pins & babs) == 0; }
    bool fetchingVector() const { return (_pins & babs) == bs; }
    bool halting() const { return (_pins & babs) == babs; }
    bool lastInstructionCycle() const { return _pins & lic; }
    bool unchanged(const Signals &prev) const {
        return _pins == prev._pins && _dbus == prev._dbus;
    }
    bool readCycle(const Signals &prev) const {
        return (prev._pins & avma) && (_pins & rw);
    }
    bool writeCycle(const Signals &prev) const {
        return (prev._pins & avma) && (_pins & rw) == 0;
    }

private:
    enum : uint8_t {
        bs = _BV(0),
        ba = _BV(1),
        babs = _BV(1) | _BV(0),
        reset = _BV(2),
        halt = _BV(3),
        lic = _BV(4),
        avma = _BV(5),
        rw = _BV(6),
    };
    uint8_t _pins;
    uint8_t _dbus;
};

class Pins {
public:
    void begin();
    void loop();
    bool isRunning() const { return _freeRunning; }

    void print() const;
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
    void leaveIoRequest();
    uint16_t ioRequestAddress() const;
    bool ioRequestWrite() const;
    uint8_t ioGetData();
    void ioSetData(uint8_t data);

    static constexpr uint16_t ioBaseAddress() { return 0xFFC0; }

    void assertIrq(const uint8_t mask);
    void negateIrq(const uint8_t mask = 0xff);
    uint8_t irqSignals(const uint8_t mask = 0xff) const { return _irq & mask; }
    static uint8_t getIrqMask(uint16_t addr) {
        return 1 << (addr - ioBaseAddress());
    }

    int sdCardChipSelectPin() const;

private:
    void cycle();
    void setData(uint8_t data);
    void unhalt();

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
        uint8_t _dir = INPUT;
        uint8_t _data;
        bool _valid;
        bool _capture;
    };

    bool _freeRunning;
    bool _stopRunning;
    Signals _signals;
    Signals _previous;
    Dbus _dbus;
    uint8_t _irq;
};

extern Pins Pins;

#endif /* __PINS_H__ */
