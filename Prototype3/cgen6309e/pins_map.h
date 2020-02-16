/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __pins_map_h__
#define __pins_map_h__

#include <avr/io.h>

// 0xFFC0 ~ 0xFFDF
#define IO_ADR_BASE 0xFFC0

/* Bit pattern of ADRn of IO_ADR_BASE */
#define IOA(n) (((IO_ADR_BASE >> (n)) & 1) << ADR##n)
/* Bit position of ADRn */
#define BVA(n) _BV(ADR##n)

/* ATmega328P 28pin 20MHz external */
#if defined(ARDUINO_AVR_ATmega328)

#define ADRH_PORT  D
#define ADR15  7
#define ADR14  6
#define ADR13  5
#define ADR12  4
#define ADR11  3
#define ADR10  2
#define ADR9   1
#define ADR5   0
#define IO_ADRH    (IOA(15)|IOA(14)|IOA(13)|IOA(12)|IOA(11)|IOA(10)|IOA(9)|IOA(5))
#define ADRH_BUS   (BVA(15)|BVA(14)|BVA(13)|BVA(12)|BVA(11)|BVA(10)|BVA(9)|BVA(5))

#define ADRM_PORT  B
#define ADR8   0
#define ADR7   5
#define ADR6   1
#define ADR4   2
#define ADR3   3
#define ADR2   4
#define IO_ADRM    (IOA(8)|IOA(7)|IOA(6)|IOA(4)|IOA(3)|IOA(2))
#define ADRM_BUS   (BVA(8)|BVA(7)|BVA(6)|BVA(4)|BVA(3)|BVA(2))

#define CLK_E_PORT C
#define CLK_E_PIN  4

#define CLK_Q_PORT C
#define CLK_Q_PIN  5

#define INT_PORT   C
#define INT_PIN    3

#define ACK_PORT   C
#define ACK_PIN    2

#define STEP_PORT  C
#define STEP_PIN   1

#endif

#define __concat2__(a,b) a##b

#define __PIN__(name)  name##_PIN
#define __BUS__(name)  name##_BUS
#define __PORT__(name) name##_PORT
#define __PORT_DDR__(port) __concat2__(DDR,port)
#define __PORT_OUT__(port) __concat2__(PORT,port)
#define __PORT_IN__(port)  __concat2__(PIN,port)
#define PDIR(name) __PORT_DDR__(__PORT__(name))
#define POUT(name) __PORT_OUT__(__PORT__(name))
#define PIN(name)  __PORT_IN__(__PORT__(name))
#define PIN_m(name)   _BV(__PIN__(name))
#define BUS_gm(name)  __BUS__(name)

#endif
