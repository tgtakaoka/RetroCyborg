#ifndef __pins_map_h__
#define __pins_map_h__

#include <stdint.h>

#if defined(ARDUINO_AVR_ITSYBITSY32U4_5V)
/* Adafruit ItsyBitsy 32u4 5V */

#define HALT_BV    PF7
#define HALT_PIN   PINF
#define HALT_PORT  PORTF
#define HALT_DDR   DDRF

#define BS_BV      PF6
#define BS_PIN     PINF
#define BS_PORT    PORTF
#define BS_DDR     DDRF

#define BA_BV      PF5
#define BA_PIN     PINF
#define BA_PORT    PORTF
#define BA_DDR     DDRF

#define LIC_BV     PF4
#define LIC_PIN    PINF
#define LIC_PORT   PORTF
#define LIC_DDR    DDRF

#define AVMA_BV    PF1
#define AVMA_PIN   PINF
#define AVMA_PORT  PORTF
#define AVMA_DDR   DDRF

#define BUSY_BV    PF0
#define BUSY_PIN   PINF
#define BUSY_PORT  PORTF
#define BUSY_DDR   DDRF

#define RESET_BV   PB1
#define RESET_PIN  PINB
#define RESET_PORT PORTB
#define RESET_DDR  DDRB

#define STEP_BV    PB2
#define STEP_PIN   PINB
#define STEP_PORT  PORTB
#define STEP_DDR   DDRB

#define ACK_BV     PB3
#define ACK_PIN    PINB
#define ACK_PORT   PORTB
#define ACK_DDR    DDRB

#define RD_WR_BV   PB4
#define RD_WR_PIN  PINB
#define RD_WR_PORT PORTB
#define RD_WR_DDR  DDRB

#define RAM_E_BV   PB7
#define RAM_E_PIN  PINB
#define RAM_E_PORT PORTB
#define RAM_E_DDR  DDRB

#define DB0_BV     PB6
#define DB0_PIN    PINB
#define DB0_PORT   PORTB
#define DB0_DDR    DDRB

#define DB1_BV     PB5
#define DB1_PIN    PINB
#define DB1_PORT   PORTB
#define DB1_DDR    DDRB

#define DB2_BV     PE6
#define DB2_PIN    PINE
#define DB2_PORT   PORTE
#define DB2_DDR    DDRE

#define DB3_BV     PC6
#define DB3_PIN    PINC
#define DB3_PORT   PORTC
#define DB3_DDR    DDRC

#define DB4_BV     PD0
#define DB4_PIN    PIND
#define DB4_PORT   PORTD
#define DB4_DDR    DDRD

#define DB5_BV     PD1
#define DB5_PIN    PIND
#define DB5_PORT   PORTD
#define DB5_DDR    DDRD

#define DB6_BV     PD3
#define DB6_PIN    PIND
#define DB6_PORT   PORTD
#define DB6_DDR    DDRD

#define DB7_BV     PD2
#define DB7_PIN    PIND
#define DB7_PORT   PORTD
#define DB7_DDR    DDRD

#endif

#endif
