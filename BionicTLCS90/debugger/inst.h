#ifndef __INST_H__
#define __INST_H__

#include <stdint.h>

#include "signals.h"

struct Inst {
    Inst() : instLen(0), busCycles(0), sequence(0) {}

    uint8_t instLen;    // Instruction length in byte
    uint8_t busCycles;  // The number of bus cycles other than instrruction and
                        // operands
    uint8_t prefetch;   // The prefetch bus cycle position from instruction or
                        // zero

    bool get(uint16_t addr);
    bool match(
            const Signals &p, const Signals &end, const Signals *pre = nullptr);

    static constexpr uint8_t HALT = 0x01;
    static constexpr uint8_t SWI = 0xFF;

private:
    uint8_t sequence;
    bool busMatch(const char *seq, uint16_t instAddr, const Signals &p,
            const Signals &end) const;
};

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
