#ifndef __PINS_MAP_H__
#define __PINS_MAP_H__

#include <avr/io.h>

// 0xFFC0 ~ 0xFFDF
#define IO_ADR_BASE 0xFFC0

/* Bit pattern of ADRn of IO_ADR_BASE */
#define IOA(n) (((IO_ADR_BASE >> (n)) & 1) << ADR##n)
/* Bit position of ADRn */
#define BVA(n) _BV(ADR##n)

/* ATtinyXX16 20pin 20MHz internal */
#if defined(__AVR_ATtiny3216__) \
 || defined(__AVR_ATtiny1616__) \
 || defined(__AVR_ATtint816__) \
 || defined(__AVR_ATtiny416__)

#define ADRH_PORT  A
#define ADR15  3
#define ADR14  2
#define ADR13  1
#define ADR12  7
#define ADR11  6
#define ADR10  5
#define ADR5   4
#define IO_ADRH    (IOA(15)|IOA(14)|IOA(13)|IOA(12)|IOA(11)|IOA(10)|IOA(5))
#define ADRH_BUS   (BVA(15)|BVA(14)|BVA(13)|BVA(12)|BVA(11)|BVA(10)|BVA(5))

#define ADRM_PORT  B
#define ADR9   1
#define ADR8   2
#define ADR7   3
#define ADR6   4
#define ADR4   5
#define IO_ADRM    (IOA(9)|IOA(8)|IOA(7)|IOA(6)|IOA(4))
#define ADRM_BUS   (BVA(9)|BVA(8)|BVA(7)|BVA(6)|BVA(4))

#define CLK_E_PORT C
#define CLK_E_PIN  2

#define CLK_Q_PORT C
#define CLK_Q_PIN  3

#define INT_PORT   C
#define INT_PIN    1

#define ACK_PORT   C
#define ACK_PIN    0

#define STEP_PORT  B
#define STEP_PIN   0

#endif

#define __concat2__(a, b)    a##b
#define __concat3__(a, b, c) a##b##c

#define __PIN__(name)  name##_PIN
#define __BUS__(name)  name##_BUS
#define __PORT__(name) name##_PORT
#define __PSTRUCT__(port) __concat2__(PORT, port)
#define __PINCTRL__(pin)  __concat3__(PIN, pin, CTRL)
#define __VPOUT__(port)   __concat3__(VPORT, port, _OUT)
#define __VPIN__(port)    __concat3__(VPORT, port, _IN)
#define PORT(name)    __PSTRUCT__(__PORT__(name))
#define PINCTRL(name) PORT(name).__PINCTRL__(__PIN__(name))
#define POUT(name)    __VPOUT__(__PORT__(name))
#define PIN(name)     __VPIN__(__PORT__(name))
#define PIN_m(name)   _BV(__PIN__(name))
#define BUS_gm(name)  __BUS__(name)

#endif
