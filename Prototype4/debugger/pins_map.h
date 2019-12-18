#ifndef __pins_map_h__
#define __pins_map_h__

/* ATmega4809-P 40pin 20MHz internal */
/* Optiboot UART1 default pins       */
#if defined(ARDUINO_AVR_ATmega4809)

#define Console      Serial2
#define CONSOLE_BAUD 115200

#define HALT_PORT  D
#define HALT_PIN   0

#define IRQ_PORT   D
#define IRQ_PIN    1

#define BS_PORT    F
#define BS_PIN     0

#define BA_PORT    F
#define BA_PIN     1

#define LIC_PORT   F
#define LIC_PIN    2

#define AVMA_PORT  F
#define AVMA_PIN   3

#define RESET_PORT D
#define RESET_PIN  5

#define STEP_PORT  D
#define STEP_PIN   7

#define ACK_PORT   D
#define ACK_PIN    6

#define INT_PORT   D
#define INT_PIN    3
#define INT_INTERRUPT digitalPinToInterrupt(11)

#define USR_SW_PORT D
#define USR_SW_PIN  2
#define USR_SW_INTERRUPT digitalPinToInterrupt(10)

#define USR_LED_PORT D
#define USR_LED_PIN  4

#define RD_WR_PORT F
#define RD_WR_PIN  4

#define RAM_E_PORT F
#define RAM_E_PIN  5

#define ADR0_PORT  C
#define ADR0_PIN   4

#define ADR1_PORT  C
#define ADR1_PIN   5

#define DB_PORT    A
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
#define PIN_m(name)   _BV(__PIN__(name))
#define BUS_gm(name)  __BUS__(name)

#endif
