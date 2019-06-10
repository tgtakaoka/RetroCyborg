#ifndef __PINS_MAP_H__
#define __PINS_MAP_H__

#include <avr/io.h>

/* Bit pattern of ADRn of IO_ADR_BASE */
#define IOA(n) (((IO_ADR_BASE >> (n)) & 1) << ADR##n)
/* Bit position of ADRn */
#define BVA(n) _BV(ADR ## n)

/* ATmega328P 28pin 20MHz external */
#if defined(ARDUINO_AVR_ATmega328)

#define ADR15  PD7
#define ADR14  PD6
#define ADR13  PD5
#define ADR12  PD4
#define ADR11  PD3
#define ADR10  PD2
#define ADR9   PD1
#define ADR5   PD0
#define IO_ADRH    (IOA(15)|IOA(14)|IOA(13)|IOA(12)|IOA(11)|IOA(10)|IOA(9)|IOA(5))
#define ADRH_BUS   (BVA(15)|BVA(14)|BVA(13)|BVA(12)|BVA(11)|BVA(10)|BVA(9)|BVA(5))
#define ADRH_PIN   PIND
#define ADRH_PORT  PORTD
#define ADRH_DDR   DDRD

#define ADR8   PB0
#define ADR7   PB5
#define ADR6   PB1
#define ADR4   PB2
#define ADR3   PB3
#define ADR2   PB4
#define IO_ADRM    (IOA(8)|IOA(7)|IOA(6)|IOA(4)|IOA(3)|IOA(2))
#define ADRM_BUS   (BVA(8)|BVA(7)|BVA(6)|BVA(4)|BVA(3)|BVA(2))
#define ADRM_PIN   PINB
#define ADRM_PORT  PORTB
#define ADRM_DDR   DDRB

#define CLK_E_BV   PC4
#define CLK_E_PIN  PINC
#define CLK_E_PORT PORTC
#define CLK_E_DDR  DDRC

#define CLK_Q_BV   PC5
#define CLK_Q_PIN  PINC
#define CLK_Q_PORT PORTC
#define CLK_Q_DDR  DDRC

#define INT_BV     PC3
#define INT_PINC   PINC
#define INT_PORT   PORTC
#define INT_DDR    DDRC

#define ACK_BV     PC2
#define ACK_PIN    PINC
#define ACK_PORT   PORTC
#define ACK_DDR    DDRC

#define STEP_BV    PC1
#define STEP_PIN   PINC
#define STEP_PORT  PORTC
#define STEP_DDR   DDRC

#endif

#endif
