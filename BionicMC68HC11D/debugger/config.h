#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define MPU_MODE 3 /* Normal expanded mode */

#define VERSION_TEXT "0.1"
#define Console Serial

#define ACIA_BASE_ADDR 0xDF00

#if defined(ARDUINO_TEENSY41)
#define PORT_AD 6     /* GPIO6 */
#define AD_gp 16      /* P6.16 */
#define AD_gm 0xFF    /* P6.16-P6.23 */
#define AD_vp 0       /* AD0-AD7 */
#define PIN_AD0 19    /* P6.16 */
#define PIN_AD1 18    /* P6.17 */
#define PIN_AD2 14    /* P6.18 */
#define PIN_AD3 15    /* P6.19 */
#define PIN_AD4 40    /* P6.20 */
#define PIN_AD5 41    /* P6.21 */
#define PIN_AD6 17    /* P6.22 */
#define PIN_AD7 16    /* P6.23 */
#define PORT_AH 6     /* GPIO6 */
#define AH_gp 24      /* P6.24-P6.31 */
#define AH_gm 0xFF    /* P6.00-P6.07 */
#define AH_vp 8       /* A8-A15 */
#define PIN_AH8 22    /* P6.24 */
#define PIN_AH9 23    /* P6.25 */
#define PIN_AH10 20   /* P6.26 */
#define PIN_AH11 21   /* P6.27 */
#define PIN_AH12 38   /* P6.28 */
#define PIN_AH13 39   /* P6.29 */
#define PIN_AH14 26   /* P6.30 */
#define PIN_AH15 27   /* P6.31 */
#define PIN_AS 2      /* P9.04 */
#define PIN_RW 3      /* P9.05 */
#define PIN_IRQ 4     /* P9.06 */
#define PIN_XIRQ 33   /* P9.07 */
#define PIN_SCITXD 0  /* P6.03 */
#define PIN_SCIRXD 1  /* P6.02 */
#define PIN_EXTAL 5   /* P9.08 */
#define PIN_E 29      /* P9.31 */
#define PIN_MODA 6    /* P7.10 */
#define PIN_LIR 6     /* P7.10 */
#define PIN_MODB 9    /* P7.11 */
#define PIN_RESET 28  /* P8.18 */
#define PIN_USRSW 24  /* P6.12 */
#define PIN_USRLED 25 /* P6.13 */
#endif

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
