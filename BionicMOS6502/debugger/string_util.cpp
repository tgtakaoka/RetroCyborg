#include "string_util.h"

static char toHex4(uint8_t val) {
    val &= 0xf;
    return val < 10 ? val + '0' : val - 10 + 'A';
}

void outHex8(char *p, uint8_t val) {
    p[0] = toHex4(val >> 4);
    p[1] = toHex4(val);
}

void outHex16(char *p, uint16_t val) {
    outHex8(p + 0, static_cast<uint8_t>(val >> 8));
    outHex8(p + 2, static_cast<uint8_t>(val));
}

void outHex24(char *p, uint32_t val) {
    outHex8(p + 0, static_cast<uint8_t>(val >> 16));
    outHex16(p + 2, static_cast<uint16_t>(val));
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
