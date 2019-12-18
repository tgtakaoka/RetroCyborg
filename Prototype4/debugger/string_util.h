/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __console_h__
#define __console_h__

#include <stdint.h>

char *outText(char *p, const char *text);
char *outHex8(char *p, uint8_t value);
char *outHex16(char *p, uint16_t value);

#endif
