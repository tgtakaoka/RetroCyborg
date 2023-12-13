#include "string_util.h"

namespace debugger {

namespace {
char toHex4(uint8_t val) {
    val &= 0xf;
    return val < 10 ? val + '0' : val - 10 + 'A';
}
}  // namespace

void outHex4(char *p, uint8_t val) {
    *p = toHex4(val);
}

void outHex8(char *p, uint8_t val) {
    outHex4(p + 0, val >> 4);
    outHex4(p + 1, val);
}

void outHex16(char *p, uint16_t val) {
    outHex8(p + 0, static_cast<uint8_t>(val >> 8));
    outHex8(p + 2, static_cast<uint8_t>(val));
}

void outHex20(char *p, uint32_t val) {
    outHex4(p + 0, static_cast<uint8_t>(val >> 16));
    outHex16(p + 1, static_cast<uint16_t>(val));
}

void outHex24(char *p, uint32_t val) {
    outHex8(p + 0, static_cast<uint8_t>(val >> 16));
    outHex16(p + 2, static_cast<uint16_t>(val));
}

void outHex32(char *p, uint32_t val) {
    outHex16(p + 0, static_cast<uint16_t>(val >> 16));
    outHex16(p + 4, static_cast<uint16_t>(val));
}

}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
