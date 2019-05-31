#include <avr/io.h>

// 0xFFC0 ~ 0xFFDF
#define IO_ADR_BASE 0xFFC0

#include "pins.h"
#include "pins_map.h"

class Pins Pins;

#if F_CPU == 20000000
#define nop() do { \
  __asm__ __volatile__ ("nop"); \
  __asm__ __volatile__ ("nop"); \
  __asm__ __volatile__ ("nop"); \
} while (0)
#endif
#if F_CPU == 16000000
#define nop() do { \
  __asm__ __volatile__ ("nop"); \
  __asm__ __volatile__ ("nop"); \
} while (0)
#endif

void Pins::begin() {
#if defined(PORTA_DIR)
  DDRA = PORTA_DIR;
  PORTB = PORTB_INIT | PORTB_PULL;
#endif
#if defined(PORTB_DIR)
  DDRB = PORTB_DIR;
  PORTB = PORTB_INIT | PORTB_PULL;
#endif
#if defined(PORTC_DIR)
  DDRC = PORTC_DIR;
  PORTC = PORTC_INIT | PORTC_PULL;
#endif
#if defined(PORTD_DIR)
  DDRD = PORTD_DIR;
  PORTD = PORTD_INIT | PORTD_PULL;
#endif
}

bool Pins::isIoAddr() const {
  return ::isIoAddr();
}

bool Pins::isAck() const {
  return (PIN_ACK & _BV(ACK)) == 0;
}

bool Pins::isStep() const {
  return (PIN_STEP & _BV(STEP)) == 0;
}

void Pins::setE() {
  PORT_CLK_E |= _BV(CLK_E);
  nop();
}

void Pins::clrE() {
  PORT_CLK_E &= ~_BV(CLK_E);
}

void Pins::setQ() {
  PORT_CLK_Q |= _BV(CLK_Q);
  nop();
}

void Pins::clrQ() {
  PORT_CLK_Q &= ~_BV(CLK_Q);
}

void Pins::assertInt() {
  PORT_INT &= ~_BV(INT);
}

void Pins::negateInt() {
  PORT_INT |= _BV(INT);
}
