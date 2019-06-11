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

#define ADR8   PB5
#define ADR7   PB4
#define ADR6   PB3
#define ADR4   PB2
#define ADR3   PB1
#define ADR2   PB0
#define IO_ADRM    (IOA(8)|IOA(7)|IOA(6)|IOA(4)|IOA(3)|IOA(2))
#define ADRM_BUS   (BVA(8)|BVA(7)|BVA(6)|BVA(4)|BVA(3)|BVA(2))
#define ADRM_PIN   PINB
#define ADRM_PORT  PORTB
#define ADRM_DDR   DDRB

#define CLK_E_BV   PC1
#define CLK_E_PIN  PINC
#define CLK_E_PORT PORTC
#define CLK_E_DDR  DDRC

#define CLK_Q_BV   PC0
#define CLK_Q_PIN  PINC
#define CLK_Q_PORT PORTC
#define CLK_Q_DDR  DDRC

#define INT_BV     PC2
#define INT_PINC   PINC
#define INT_PORT   PORTC
#define INT_DDR    DDRC

#define ACK_BV     PC3
#define ACK_PIN    PINC
#define ACK_PORT   PORTC
#define ACK_DDR    DDRC

#define STEP_BV    PC4
#define STEP_PIN   PINC
#define STEP_PORT  PORTC
#define STEP_DDR   DDRC

#endif

/* ATtiny2313 20pin 20MHz external, disable RESET */
#if defined(ARDUINO_AVR_ATTINYX313)

#define ADR15  PB7
#define ADR14  PB6
#define ADR13  PB5
#define ADR12  PB4
#define ADR11  PB3
#define ADR10  PB2
#define ADR9   PB1
#define ADR5   PB0
#define IO_ADRH    (IOA(15)|IOA(14)|IOA(13)|IOA(12)|IOA(11)|IOA(10)|IOA(9)|IOA(5))
#define ADRH_BUS   (BVA(15)|BVA(14)|BVA(13)|BVA(12)|BVA(11)|BVA(10)|BVA(9)|BVA(5))
#define ADRH_PIN   PINB
#define ADRH_PORT  PORTB
#define ADRH_DDR   DDRB

#define ADR8   PD6
#define ADR7   PD5
#define IO_ADRM    (IOA(8)|IOA(7))
#define ADRM_BUS   (BVA(8)|BVA(7))
#define ADRM_PIN   PIND
#define ADRM_PORT  PORTD
#define ADRM_DDR   DDRD

#define ADR6   PA2
#define IO_ADRL    IOA(6)
#define ADRL_BUS   BVA(6)
#define ADRL_PIN   PINA
#define ADRL_PORT  PORTA
#define ADRL_DDR   DDRA

#define CLK_E_BV   PD1
#define CLK_E_PIN  PIND
#define CLK_E_PORT PORTD
#define CLK_E_DDR  DDRD

#define CLK_Q_BV   PD0
#define CLK_Q_PIN  PIND
#define CLK_Q_PORT PORTD
#define CLK_Q_DDR  DDRD

#define INT_BV     PD2
#define INT_PINC   PIND
#define INT_PORT   PORTD
#define INT_DDR    DDRD

#define ACK_BV     PD3
#define ACK_PIN    PIND
#define ACK_PORT   PORTD
#define ACK_DDR    DDRD

#define STEP_BV    PD4
#define STEP_PIN   PIND
#define STEP_PORT  PORTD
#define STEP_DDR   DDRD

#endif

/* ATtiny85 8pin 16MHz PLL, disable RESET */
#if defined(ARDUINO_AVR_ATTINYX5)

#define IOR   PB5
#define IO_ADRH    0
#define ADRH_BUS   _BV(IOR)
#define ADRH_PIN   PINB
#define ADRH_PORT  PORTB
#define ADRH_DDR   DDRB

#define CLK_E_BV   PB1
#define CLK_E_PIN  PINB
#define CLK_E_PORT PORTB
#define CLK_E_DDR  DDRB

#define CLK_Q_BV   PB0
#define CLK_Q_PIN  PINB
#define CLK_Q_PORT PORTB
#define CLK_Q_DDR  DDRB

#define INT_BV     PB2
#define INT_PINC   PINB
#define INT_PORT   PORTB
#define INT_DDR    DDRB

#define ACK_BV     PB3
#define ACK_PIN    PINB
#define ACK_PORT   PORTB
#define ACK_DDR    DDRB

#define STEP_BV    PB4
#define STEP_PIN   PINB
#define STEP_PORT  PORTB
#define STEP_DDR   DDRB

#endif

#endif
