#ifndef __DIGITAL_FAST_H__
#define __DIGITAL_FAST_H__

#define __concat2__(a, b) a##b

#define __GPIO__(port) __concat2__(GPIO, port)
#define __DDR__(port) __concat2__(port, _PDDR)
#define __OUT__(port) __concat2__(port, _PDOR)
#define __SET__(port) __concat2__(port, _PSOR)
#define __CLR__(port) __concat2__(port, _PCOR)
#define __IN__(port) __concat2__(port, _PDIR)
#define GPIO(name) __GPIO__(PORT_##name)
#define PDDR(name) __DDR__(GPIO(name))
#define POUT(name) __OUT__(GPIO(name))
#define PSET(name) __SET__(GPIO(name))
#define PCLR(name) __CLR__(GPIO(name))
#define PIN(name) __IN__(GPIO(name))
#define BUS_gp(name) (name##_gp)
#define BUS_gm(name) (name##_gm)

#define busMode(name, mode)                                \
    do {                                                   \
        if ((mode) == INPUT) {                             \
            PDDR(name) &= ~(BUS_gm(name) << BUS_gp(name)); \
        } else {                                           \
            PDDR(name) |= (BUS_gm(name) << BUS_gp(name));  \
        }                                                  \
    } while (0)
#define busRead(name) ((PIN(name) & BUS_gm(name)) << BUS_gp(name))
#define busWrite(name, val)                                                   \
    do {                                                                      \
        const uint32_t v = static_cast<uint32_t>(val & 0xFF) << BUS_gp(name); \
        const uint32_t o = POUT(name) & ~(BUS_gm(name) << BUS_gp(name));      \
        POUT(name) = o | v;                                                   \
    } while (0)

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
