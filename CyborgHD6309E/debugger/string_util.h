/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __STRING_UTIL_H__
#define __STRING_UTIL_H__

#include <stdint.h>

char *outText(char *p, const char *text);
char *outHex8(char *p, uint8_t value);
char *outHex16(char *p, uint16_t value);

#endif
