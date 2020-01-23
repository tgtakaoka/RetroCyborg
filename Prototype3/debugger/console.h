/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __console_h__
#define __console_h__

#include <stdint.h>

#define Console Serial

char *outText(char *p, const char *text);
char *outHex8(char *p, uint8_t value, char suffix = 0);
char *outHex16(char *p, uint16_t value, char suffix = 0);
void printHex8(uint8_t value, char suffix = 0);
void printHex16(uint16_t value, char suffix = 0);

#endif
