#ifndef __MC6801_SCI_HANDLER_H__
#define __MC6801_SCI_HANDLER_H__

#include "serial_handler.h"

namespace debugger {
namespace mc6801 {

struct Mc6801SciHandler final : SerialHandler {
    Mc6801SciHandler();

    const char *name() const override;
    const char *description() const override;
    uint32_t baseAddr() const override { return ADDR_RMCR; }
    bool isSelected(uint32_t addr) const override;
    void write(uint32_t addr, uint16_t data) override;

protected:
    uint8_t _rmcr;  // RMCR($10): Rate and Mode Control Register
    static constexpr uint16_t ADDR_RMCR = 0x0010;
    static constexpr uint8_t RMCR_SS_gm = 0x03;  // Speed Select
    static constexpr uint8_t RMCR_CC_gm = 0x0C;  // Clock source Control

    void resetHandler() override;
};

extern struct Mc6801SciHandler SciH;

}  // namespace mc6801
}  // namespace debugger
#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
