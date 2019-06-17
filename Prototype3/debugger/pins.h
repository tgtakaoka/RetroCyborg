#ifndef __pins_h__
#define __pins_h__

#include <stdint.h>

class Pins {

public:
  void begin();
  void reset();
  void print() const;

  uint8_t dbus() { return _signals.dbus; }
  void execInst(const uint8_t *inst, uint8_t len, bool show = false);
  void captureWrites(const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max);
  void run();
  void halt(bool show = false);
  void step(bool show = false);

private:

  struct Status {
    void get();
    enum {
      reset = _BV(0),
      halt  = _BV(1),
      ba    = _BV(2),
      bs    = _BV(3),
      lic   = _BV(4),
      avma  = _BV(5),
      rw    = _BV(6),
    };
    uint8_t pins;
    uint8_t dbus;
    bool inst;
  };

  class Dbus {
  public:
    void begin();
    void set(uint8_t data);
    void output();
    void input();
    bool valid() const { return _valid; }
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

  Status _signals;
  Status _previous;
  Dbus _dbus;
};

extern Pins Pins;

#endif /* __pins_h__ */
