/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __console_h__
#define __console_h__

#include <stdint.h>

#define Console Serial

void printHex8(uint8_t value, char suffix = 0);
void printHex16(uint16_t value, char suffix = 0);

#endif
