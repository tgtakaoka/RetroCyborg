#ifndef __CDP1802_SCI_HANDLER_H__
#define __CDP1802_SCI_HANDLER_H__

#include "serial_handler.h"

namespace debugger {
namespace cdp1802 {

struct Cdp1802SciHandler final : SerialHandler {
    Cdp1802SciHandler();
    const char *name() const override;
    const char *description() const override;
    uint32_t baseAddr() const override;

protected:
    void resetHandler() override;
};

extern struct Cdp1802SciHandler SciH;

}  // namespace cdp1802
}  // namespace debugger
#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
