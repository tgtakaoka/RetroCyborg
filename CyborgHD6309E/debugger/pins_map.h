#ifndef __pins_map_h__
#define __pins_map_h__

/* ATmega4809-P 40pin 20MHz internal */
/* Optiboot UART1 default pins       */
#if defined(ARDUINO_AVR_ATmega4809)

#define Console      Serial1
#define CONSOLE_BAUD 115200

#define HALT_PORT  A
#define HALT_PIN   7

#define IRQ_PORT   A
#define IRQ_PIN    6

#define BS_PORT    A
#define BS_PIN     0

#define BA_PORT    A
#define BA_PIN     1

#define LIC_PORT   A
#define LIC_PIN    4

#define AVMA_PORT  A
#define AVMA_PIN   3

#define RESET_PORT A
#define RESET_PIN  5

#define STEP_PORT  F
#define STEP_PIN   2

#define ACK_PORT   F
#define ACK_PIN    1

#define INT_PORT   F
#define INT_PIN    0
#define INT_INTERRUPT digitalPinToInterrupt(26)

#define USR_SW_PORT F
#define USR_SW_PIN  5
#define USR_SW_INTERRUPT digitalPinToInterrupt(31)

#define USR_LED_PORT F
#define USR_LED_PIN  4

#define RD_WR_PORT A
#define RD_WR_PIN  2

#define RAM_E_PORT F
#define RAM_E_PIN  3

#define ADR0_PORT  C
#define ADR0_PIN   4

#define ADR1_PORT  C
#define ADR1_PIN   5

#define DB_PORT    D
#define DB_BUS     0xff

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
#define PIN_bm(name)  _BV(__PIN__(name))
#define BUS_gm(name)  __BUS__(name)

#endif
