#ifndef __PINS_MAP_H__
#define __PINS_MAP_H__

#include <avr/io.h>

/* ATtiny2313 20pin 20MHz external */
#if defined(ARDUINO_AVR_ATTINYX313)

#define CLK_E  PD1
#define CLK_Q  PD0
#define INT    PD4
#define ACK    PD3
#define STEP   PD2
#define ADR15  PB7
#define ADR14  PB6
#define ADR13  PB5
#define ADR12  PB4
#define ADR11  PB3
#define ADR10  PB2
#define ADR9   PB1
#define ADR8   PD6
#define ADR7   PD5
#define ADR6   PB0

#define IOA(n) (((IO_ADR_BASE >> (n)) & 1) << ADR##n)

#define IO_ADRH (IOA(15)|IOA(14)|IOA(13)|IOA(12)|IOA(11)|IOA(10)|IOA(9)|IOA(6))
#define ADRH   (_BV(ADR15)|_BV(ADR14)|_BV(ADR13)|_BV(ADR12)|_BV(ADR11)|_BV(ADR10)|_BV(ADR9)|_BV(ADR6))
#define IO_ADRL (IOA(8)|IOA(7))
#define ADRL   (_BV(ADR8)|_BV(ADR7))

#define PORT_CLK_E PORTD
#define PORT_CLK_Q PORTD
#define PORT_INT   PORTD
#define PIN_ACK    PIND
#define PIN_STEP   PIND
#define PIN_ADDRH  PINB
#define PIN_ADDRL  PIND

#define PORTB_DIR  0x00
#define PORTB_INIT 0x00
#define PORTB_PULL ADRH
#define PORTD_DIR (_BV(CLK_E)|_BV(CLK_Q)|_BV(INT))
#define PORTD_INIT _BV(INT)
#define PORTD_PULL (_BV(ACK)|_BV(STEP)|ADRL)

static inline bool isIoAddr() {
  return
      (PIN_ADDRH & ADRH) == IO_ADRH &&
      (PIN_ADDRL & ADRL) == IO_ADRL;
}

#endif

/* ATtiny85 8pin 16MHz PLL */
#if defined(ARDUINO_AVR_ATTINYX5)
#define CLK_E  PB1
#define CLK_Q  PB0
#define INT    PB2
#define ACK    PB3
#define STEP   PB4
#define IOR    PB5

#define PORT_CLK_E PORTB
#define PORT_CLK_Q PORTB
#define PORT_INT   PORTB
#define PIN_ACK    PINB
#define PIN_STEP   PINB
#define PIN_IOR    PINB

#define PORTB_DIR (_BV(CLK_E)|_BV(CLK_Q)|_BV(INT))
#define PORTB_INIT _BV(INT)
#define PORTB_PULL (_BV(ACK)|_BV(STEP|_BV(IOR)))

static inline bool isIoAddr() {
  return (PIN_IOR & IOR) == 0;
}
#endif

#endif
