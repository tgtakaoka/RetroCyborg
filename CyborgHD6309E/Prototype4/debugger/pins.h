/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __PINS_H__
#define __PINS_H__

#include <Arduino.h>
#include <stdint.h>

#include "config.h"

//#define DEBUG_SIGNALS

class Signals {
public:
    void get();
    void print(const Signals *prev = nullptr) const;
    uint8_t dbus() const { return _dbus; }
    bool running() const { return (_pins & babs) == 0; }
    bool fetchingVector() const { return (_pins & babs) == bs; }
    bool halting() const { return (_pins & babs) == babs; }
    bool lastInstructionCycle() const { return _pins & lic; }
    bool advancedValidMemoryAddress() const { return _pins & avma; }
    bool readCycle(const Signals *prev) const {
        return prev && prev->advancedValidMemoryAddress() && (_pins & rw);
    }
    bool writeCycle(const Signals *prev) const {
        return prev && prev->advancedValidMemoryAddress() && (_pins & rw) == 0;
    }
    void debug(char c) {
        (void)c;
#ifdef DEBUG_SIGNALS
        _debug = c;
#endif
    }

private:
    enum : uint8_t {
#ifdef SIGNALS_BUS
        bs = _BV(BS_PIN),
        ba = _BV(BA_PIN),
        babs = _BV(BA_PIN) | _BV(BS_BIN),
        reset = _BV(RESET_PIN),
        halt = _BV(HALT_PIN),
        lic = _BV(LIC_PIN),
        avma = _BV(AVMA_PIN),
        rw = _BV(RD_WR_PIN),
        busy = _BV(BUSY_PIN),
#else
        bs = _BV(0),
        ba = _BV(1),
        babs = _BV(1) | _BV(0),
        reset = _BV(2),
        halt = _BV(3),
        lic = _BV(4),
        avma = _BV(5),
        rw = _BV(6),
        busy = _BV(7),
#endif
    };
    uint8_t _pins;
    uint8_t _dbus;
#ifdef DEBUG_SIGNALS
    char _debug;
#endif
};

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

    Signals &cycle(const Signals *prev);
    const Signals *unhalt();
    void setData(uint8_t data);
    void printCycles() const;
    uint8_t execute(const uint8_t *inst, uint8_t len, uint8_t *capBuf,
            uint8_t max, bool capWrite, bool capRead, bool show);
    Signals &currCycle() { return _signals[_cycles]; }
    void resetCycles() { _cycles = 0; }
    void nextCycle() {
        if (_cycles < MAX_CYCLES)
            _cycles++;
    }

    static const uint8_t MAX_CYCLES = 40;
    uint8_t _cycles;
    Dbus _dbus;
    bool _freeRunning;
    bool _stopRunning;
    uint8_t _irq;
    Signals _signals[MAX_CYCLES + 1];
};

extern Pins Pins;

#endif /* __PINS_H__ */
