#include "target.h"

#include "devs_z86.h"
#include "mems_z86.h"
#include "pins_z86.h"
#include "regs_z86.h"

namespace debugger {
namespace z86 {

struct Target Targetz86 {
    "Z86C91", Pins, Regs, Memory, Devs
};

}  // namespace z86
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
