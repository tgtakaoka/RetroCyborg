/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __PINS_BASE_H__
#define __PINS_BASE_H__

#define __concat2__(a, b) a##b
#define __concat3__(a, b, c) a##b##c

#if defined(VPORTA)

#define __PIN__(name) name##_PIN
#define __BUS__(name) name##_BUS
#define __PORT__(name) name##_PORT
#define __PSTRUCT__(port) __concat2__(PORT, port)
#define __PINCTRL__(pin) __concat3__(PIN, pin, CTRL)
#define __VPOUT__(port) __concat3__(VPORT, port, _OUT)
#define __VPIN__(port) __concat3__(VPORT, port, _IN)
#define PORT(name) __PSTRUCT__(__PORT__(name))
#define PINCTRL(name) PORT(name).__PINCTRL__(__PIN__(name))
#define POUT(name) __VPOUT__(__PORT__(name))
#define PIN(name) __VPIN__(__PORT__(name))
#define PIN_bm(name) _BV(__PIN__(name))
#define BUS_gm(name) __BUS__(name)

#define pinMode(name, mode)                     \
    do {                                        \
        if ((mode) == INPUT) {                  \
            PORT(name).DIRCLR = PIN_bm(name);   \
            PINCTRL(name) &= ~PORT_PULLUPEN_bm; \
        }                                       \
        if ((mode) == INPUT_PULLUP) {           \
            PORT(name).DIRCLR = PIN_bm(name);   \
            PINCTRL(name) |= PORT_PULLUPEN_bm;  \
        }                                       \
        if ((mode) == OUTPUT)                   \
            PORT(name).DIRSET = PIN_bm(name);   \
    } while (0)
#define pinModeInvert(name) PINCTRL(name) |= PORT_INVEN_bm
#define digitalRead(name) (PIN(name) & PIN_bm(name))
#define digitalWrite(name, val)               \
    do {                                      \
        if ((val) == LOW) {                   \
            PORT(name).OUTCLR = PIN_bm(name); \
        } else {                              \
            PORT(name).OUTSET = PIN_bm(name); \
        }                                     \
    } while (0)

static void enablePullup(register8_t *pinctrl, uint8_t mask)
        __attribute__((unused));
static void enablePullup(register8_t *pinctrl, uint8_t mask) {
    while (mask) {
        if (mask & 1)
            *pinctrl |= PORT_PULLUPEN_bm;
        pinctrl++;
        mask >>= 1;
    }
}
#define busMode(name, mode)                                   \
    do {                                                      \
        if ((mode) == INPUT) {                                \
            PORT(name).DIRCLR = BUS_gm(name);                 \
        } else if ((mode) == INPUT_PULLUP) {                  \
            PORT(name).DIRCLR = BUS_gm(name);                 \
            enablePullup(&PORT(name).PIN0CTRL, BUS_gm(name)); \
        } else if ((mode) == OUTPUT) {                        \
            PORT(name).DIRSET = BUS_gm(name);                 \
        }                                                     \
    } while (0)
#define busRead(name) (PORT(name).IN & BUS_gm(name))
#define busWrite(name, val)                          \
    do {                                             \
        if (BUS_gm(name) == 0xFF) {                  \
            PORT(name).OUT = (val);                  \
        } else {                                     \
            PORT(name).OUTSET = (val)&BUS_gm(name);  \
            PORT(name).OUTCLR = ~(val)&BUS_gm(name); \
        }                                            \
    } while (0);

#else

#define __PIN__(name) name##_PIN
#define __BUS__(name) name##_BUS
#define __PORT__(name) name##_PORT
#define __PORT_DDR__(port) __concat2__(DDR, port)
#define __PORT_OUT__(port) __concat2__(PORT, port)
#define __PORT_IN__(port) __concat2__(PIN, port)
#define PDIR(name) __PORT_DDR__(__PORT__(name))
#define POUT(name) __PORT_OUT__(__PORT__(name))
#define PIN(name) __PORT_IN__(__PORT__(name))
#define PIN_m(name) _BV(__PIN__(name))
#define BUS_gm(name) __BUS__(name)

#define pinMode(name, mode)             \
    do {                                \
        if (mode == INPUT)              \
            PDIR(name) &= ~PIN_m(name); \
        if (mode == INPUT_PULLUP)       \
            PDIR(name) &= ~PIN_m(name); \
        if (mode == INPUT_PULLUP)       \
            POUT(name) |= PIN_m(name);  \
        if (mode == OUTPUT)             \
            PDIR(name) |= PIN_m(name);  \
    } while (0)
#define digitalRead(name) (PIN(name) & PIN_m(name))
#define digitalWrite(name, val)         \
    do {                                \
        if (val == LOW)                 \
            POUT(name) &= ~PIN_m(name); \
        if (val == HIGH)                \
            POUT(name) |= PIN_m(name);  \
    } while (0)
#define busMode(name, mode)               \
    do {                                  \
        if (mode == INPUT)                \
            PDIR(name) &= ~__BUS__(name); \
        if (mode == INPUT_PULLUP)         \
            PDIR(name) &= ~__BUS__(name); \
        if (mode == INPUT_PULLUP)         \
            POUT(name) |= __BUS__(name);  \
        if (mode == OUTPUT)               \
            PDIR(name) |= __BUS__(name);  \
    } while (0)
#define busRead(name) (PIN(name) & BUS_gm(name))
#define busWrite(name, val)                                              \
    do {                                                                 \
        if (BUS_gm(name) == 0xFF) {                                      \
            POUT(name) = (val);                                          \
        } else {                                                         \
            const uint8_t out =                                          \
                    (POUT(name) & ~BUS_gm(name)) | (val & BUS_gm(name)); \
            POUT(name) = out;                                            \
        }                                                                \
    } while (0)

#endif

#endif
