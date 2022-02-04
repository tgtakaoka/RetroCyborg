#ifndef __STRING_UTIL_H__
#define __STRING_UTIL_H__

#include <stdint.h>

void outHex4(char *p, uint8_t value);
void outHex8(char *p, uint8_t value);
void outHex16(char *p, uint16_t value);
void outHex20(char *p, uint32_t value);
void outHex24(char *p, uint32_t value);
void outHex32(char *p, uint32_t value);

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
