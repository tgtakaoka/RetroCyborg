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
  void cycle();
  void run();
  void stopRunning();

private:

  class Status {
  public:
    void get();
    void print() const;
    uint8_t dbus() const { return _dbus; }
    bool unchanged(const Status &prev) const {
      return _pins == prev._pins && _dbus == prev._dbus;
    }

  private:
    enum {
      iorq   = _BV(0),
      mreq   = _BV(1),
      m1     = _BV(2),
      wait   = _BV(3),
      rd     = _BV(4),
      wr     = _BV(5),
      busreq = _BV(6),
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

  bool _freeRunning;
  Status _signals;
  Status _previous;
  Dbus _dbus;

  static void pulseClock(uint8_t numClocks);
};

extern Pins Pins;

#endif /* __pins_h__ */
