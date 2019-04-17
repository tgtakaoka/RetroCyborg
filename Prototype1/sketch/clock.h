#ifndef __clock_h__
#define __clock_h__

#include <stdint.h>

class Clock {

public:
  // return: true when clock is in free running.
  bool isRunning();
  void stop();
  void run();
  // |ms|: delay while |E| is HIGH and |Q| is LOW.
  void cycle(uint16_t ms = 1);

private:
  bool _running;
};

extern Clock Clock;

#endif
