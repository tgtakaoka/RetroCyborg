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
  uint8_t dbus() { return _signals.dbus; }
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
  void negateIrq(const uint8_t mask);
  uint8_t irqStatus(const uint8_t mask = 0xff) const {
    return _irq & mask;
  }
  static uint8_t getIrqMask(uint16_t addr) {
    return 1 << (addr - ioBaseAddress());
  }

  void attachUserSwitch(void (*isr)()) const;

private:

  struct Status {
    void get();
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
    uint8_t pins;
    uint8_t dbus;
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

  bool unchanged() const;
  bool inHalt() const;
  bool vectorFetch() const;
  bool running() const;
  bool lastInstCycle() const;
  bool writeCycle() const;
  bool readCycle() const;

  bool _freeRunning;
  Status _signals;
  Status _previous;
  Dbus _dbus;
  uint8_t _irq;
};

extern Pins Pins;

#endif /* __pins_h__ */
