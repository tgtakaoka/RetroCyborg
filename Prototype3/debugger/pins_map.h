#ifndef __pins_map_h__
#define __pins_map_h__

#if defined(ARDUINO_AVR_ATmega32) || defined(ARDUINO_AVR_ATmega1284)

#define HALT_BV    PB0
#define HALT_PIN   PINB
#define HALT_PORT  PORTB
#define HALT_DDR   DDRB

#define BS_BV      PC7
#define BS_PIN     PINC
#define BS_PORT    PORTC
#define BS_DDR     DDRC

#define BA_BV      PC5
#define BA_PIN     PINC
#define BA_PORT    PORTC
#define BA_DDR     DDRC

#define LIC_BV     PD5
#define LIC_PIN    PIND
#define LIC_PORT   PORTD
#define LIC_DDR    DDRD

#define AVMA_BV    PC4
#define AVMA_PIN   PINC
#define AVMA_PORT  PORTC
#define AVMA_DDR   DDRC

#define RESET_BV   PB2
#define RESET_PIN  PINB
#define RESET_PORT PORTB
#define RESET_DDR  DDRB

#define STEP_BV    PD7
#define STEP_PIN   PIND
#define STEP_PORT  PORTD
#define STEP_DDR   DDRD

#define ACK_BV     PD6
#define ACK_PIN    PIND
#define ACK_PORT   PORTD
#define ACK_DDR    DDRD

#define INT_BV     PD3
#define INT_PIN    PIND
#define INT_PORT   PORTD
#define INT_DDR    DDRD
#define INT_INTERRUPT digitalPinToInterrupt(11)

#define RD_WR_BV   PB3
#define RD_WR_PIN  PINB
#define RD_WR_PORT PORTB
#define RD_WR_DDR  DDRB

#define RAM_E_BV   PC6
#define RAM_E_PIN  PINC
#define RAM_E_PORT PORTC
#define RAM_E_DDR  DDRC

#define ADR0_BV    PC3
#define ADR0_PIN   PINC
#define ADR0_PORT  PORTC
#define ADR0_DDR   DDRC

#define ADR1_BV    PC2
#define ADR1_PIN   PINC
#define ADR1_PORT  PORTC
#define ADR1_DDR   DDRC

#define DB_PIN     PINA
#define DB_PORT    PORTA
#define DB_DDR     DDRA

#endif

#endif
