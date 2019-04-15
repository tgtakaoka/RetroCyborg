#ifndef __clock_h__
#define __clock_h__

bool isClockRunning();
void clock(bool enable, const char *const message);
void clockCycle(int16_t ms);

#endif
