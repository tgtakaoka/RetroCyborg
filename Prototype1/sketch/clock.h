#ifndef __clock_h__
#define __clock_h__

#include <stdint.h>

class Clock {

public:
  // return: true when clock is in free running.
  bool isRunning() const;
  void stop();
  void run();
  // |ms|: delay while |E| is HIGH and |Q| is LOW.
  void cycle(uint16_t ms = 1);
  // return: cycle counter
  uint16_t getCycle() const { return _cycle; }

private:
  bool _running;
  uint16_t _cycle;
};

extern Clock Clock;

#endif
