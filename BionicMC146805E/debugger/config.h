#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define MPU_NAME "MC146805E"

#define VERSION_TEXT "* Bionic" MPU_NAME " 0.1"
#define Console Serial

#define IO_BASE_ADDR 0x17F8

#if defined(ARDUINO_TEENSY35)
#define PORT_B D      /* GPIOD */
#define B_gp 0        /* PTD00-PTD07 */
#define B_gm 0xFF     /* PTD00-PTD07 */
#define B_vp 0        /* B0-B7 */
#define PIN_B0 2      /* PTD00 */
#define PIN_B1 14     /* PTD01 */
#define PIN_B2 7      /* PTD02 */
#define PIN_B3 8      /* PTD03 */
#define PIN_B4 6      /* PTD04 */
#define PIN_B5 20     /* PTD05 */
#define PIN_B6 21     /* PTD06 */
#define PIN_B7 5      /* PTD07 */
#define PORT_AH C     /* GPIOC */
#define AH_gp 0       /* PTC00-PTC04 */
#define AH_gm 0x1F    /* PTC00-PTC04 */
#define AH_vp 8       /* A8-A12 */
#define PIN_AH8 15    /* PTC00 */
#define PIN_AH9 22    /* PTC01 */
#define PIN_AH10 23   /* PTC02 */
#define PIN_AH11 9    /* PTC03 */
#define PIN_AH12 10   /* PTC04 */
#define PIN_AS 16     /* PTB00 */
#define PIN_RW 17     /* PTB01 */
#define PIN_IRQ 19    /* PTB02 */
#define PIN_TIMER 1   /* PTB17 */
#define PIN_OSC1 29   /* PTB18 */
#define PIN_DS 33     /* PTE24 */
#define PIN_LI 34     /* PTE25 */
#define PIN_RESET 25  /* PTA05 */
#define PIN_USRSW 31  /* PTB10 */
#define PIN_USRLED 32 /* PTB11 */
#endif

#if defined(ARDUINO_TEENSY41)
#define PORT_B 6      /* GPIO6 */
#define B_gp 16       /* P6.16-P6.23 */
#define B_gm 0xFF     /* P6.00-P6.07 */
#define B_vp 0        /* B0-B7 */
#define PIN_B0 19     /* P6.16 */
#define PIN_B1 18     /* P6.17 */
#define PIN_B2 14     /* P6.18 */
#define PIN_B3 15     /* P6.19 */
#define PIN_B4 40     /* P6.20 */
#define PIN_B5 41     /* P6.21 */
#define PIN_B6 17     /* P6.22 */
#define PIN_B7 16     /* P6.23 */
#define PORT_AH 6     /* GPIO6 */
#define AH_gp 24      /* P6.24-P6.28 */
#define AH_gm 0x1F    /* P6.24-P6.28 */
#define AH_vp 8       /* A8-A12 */
#define PIN_AH8 22    /* P6.24 */
#define PIN_AH9 23    /* P6.25 */
#define PIN_AH10 20   /* P6.26 */
#define PIN_AH11 21   /* P6.27 */
#define PIN_AH12 38   /* P6.28 */
#define PIN_AS 2      /* P9.04 */
#define PIN_RW 3      /* P9.05 */
#define PIN_IRQ 4     /* P9.06 */
#define PIN_TIMER 1   /* P6.02 */
#define PIN_OSC1 5    /* P9.08 */
#define PIN_DS 6      /* P7.10 */
#define PIN_LI 9      /* P7.11 */
#define PIN_RESET 28  /* P8.18 */
#define PIN_USRSW 35  /* P7.28 */
#define PIN_USRLED 34 /* P7.29 */
#endif

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
