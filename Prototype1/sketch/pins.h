#ifndef __pins_h__
#define __pins_h__

#include <stdint.h>

class Pins {

public:  
  class Status {
  public:
    bool inHalt() const { return !_halt && _ba && _bs; }
    bool running() const { return !_ba && !_bs; }
    bool lastInstCycle() const { return _lic && _avma; }
    bool vectorFetch() const { return !_ba && _bs; }
    bool busBusy() const { return _busy; }
    bool busWrite() const { return !_rw; }
    bool equals(const Status& o) const;
    void print(bool nl = true) const;

  private:
    friend class Pins;
    void get();

    unsigned _reset:1;
    unsigned _halt:1;
    unsigned _ba:1;
    unsigned _bs:1;
    unsigned _lic:1;
    unsigned _avma:1;
    unsigned _busy:1;
    unsigned _rw:1;
    uint8_t _dbus;
    uint16_t _cycle;
    enum {
      INST, OP, VH, VL
    } _mode;
  };

  class Dbus {
  public:
    void set(uint8_t data);
    void output();
    void input();
  private:
    uint8_t _dir = INPUT;
    uint8_t _data;
    bool _data_valid;
  };

  void begin(Status& pins);
  void cycle(Status& pins);
  // |value|: LOW or HIGH.
  void reset(uint8_t value);
  void halt(uint8_t value);
  // |vh|,|vl|: vector
  void setVector(uint8_t vh, uint8_t vl);
  // |data|: 8 bit data.
  void setData(uint8_t data) { _dbus.set(data); }
  void print(bool nl = true) const;

private:
  friend class Status;
  static uint16_t _cycle;
  Dbus _dbus;
  uint8_t _vector_high;
  uint8_t _vector_low;
};

extern Pins Pins;

#endif /* __pins_h__ */
