#ifndef __pins_h__
#define __pins_h__

#include <stdint.h>

void setupPins();
void reset(uint8_t);
void halt(uint8_t);
bool setDataBusDirection(uint8_t);
void setDataBus(uint8_t);
uint8_t getDataBus();
void printStatus();

#endif /* __pins_h__ */
