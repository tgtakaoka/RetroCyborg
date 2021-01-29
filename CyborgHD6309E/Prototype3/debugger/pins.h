/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __pins_h__
#define __pins_h__

#include <Arduino.h>
#include <stdint.h>

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
  uint8_t dbus() { return _signals.dbus(); }
  void execInst(const uint8_t *inst, uint8_t len, bool show = false);
  void captureWrites(const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max);

  void acknowledgeIoRequest();
  void leaveIoRequest();
  uint16_t ioRequestAddress() const;
  bool ioRequestWrite() const;
  uint8_t ioGetData();
  void ioSetData(uint8_t data);

  static constexpr uint16_t ioBaseAddress() { return 0xFFC0; }

  void assertIrq(const uint8_t mask);
  void negateIrq(const uint8_t mask = 0xff);
  uint8_t irqStatus(const uint8_t mask = 0xff) const {
    return _irq & mask;
  }
  static uint8_t getIrqMask(uint16_t addr) {
    return 1 << (addr - ioBaseAddress());
  }

private:

  class Status {
  public:
    void get();
    void print() const;
    uint8_t dbus() const { return _dbus; }
    bool running() const { return (_pins & babs) == 0; }
    bool fetchingVector() const { return (_pins & babs) == bs; }
    bool halting() const { return (_pins & babs) == babs; }
    bool lastInstructionCycle() const { return _pins & lic; }
    bool unchanged(const Status &prev) const {
      return _pins == prev._pins && _dbus == prev._dbus;
    }
    bool readCycle(const Status &prev) const {
      return (prev._pins & avma) && (_pins & rw);
    }
    bool writeCycle(const Status &prev) const {
      return (prev._pins & avma) && (_pins & rw) == 0;
    }

  private:
    enum {
      bs    = _BV(0),
      ba    = _BV(1),
      babs  = _BV(1) | _BV(0),
      reset = _BV(2),
      halt  = _BV(3),
      lic   = _BV(4),
      avma  = _BV(5),
      rw    = _BV(6),
    };
    uint8_t _pins;
    uint8_t _dbus;
  };

  class Dbus {
  public:
    void begin();
    void set(uint8_t data);
    void output();
    void input();
    bool valid() const {
      return _valid;
    }
    void capture(bool enable);
    static uint8_t getDbus();
  private:
    void setDbus(uint8_t dir, uint8_t data);
    uint8_t _dir = INPUT;
    uint8_t _data;
    bool _valid;
    bool _capture;
  };

  void cycle();
  void setData(uint8_t data);
  void unhalt();

  bool _freeRunning;
  bool _stopRunning;
  Status _signals;
  Status _previous;
  Dbus _dbus;
  uint8_t _irq;
};

extern Pins Pins;

#endif /* __pins_h__ */
