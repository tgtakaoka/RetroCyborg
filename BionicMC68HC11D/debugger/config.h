#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define MPU_MODE 3 /* Normal expanded mode */

#define VERSION_TEXT "0.1"
#define Console Serial

#define ACIA_BASE_ADDR 0xDF00

#if defined(ARDUINO_TEENSY35)
#define PORT_AD D     /* GPIOD */
#define AD_gp 0       /* PTD00-PTD07 */
#define AD_gm 0xFF    /* PTD00-PTD07 */
#define AD_vp 0       /* AD0-AD7 */
#define PIN_AD0 2     /* PTD00 */
#define PIN_AD1 14    /* PTD01 */
#define PIN_AD2 7     /* PTD02 */
#define PIN_AD3 8     /* PTD03 */
#define PIN_AD4 6     /* PTD04 */
#define PIN_AD5 20    /* PTD05 */
#define PIN_AD6 21    /* PTD06 */
#define PIN_AD7 5     /* PTD07 */
#define PORT_AH C     /* GPIOC */
#define AH_gp 0       /* PTC00-PTC07 */
#define AH_gm 0xFF    /* PTC00-PTC07 */
#define AH_vp 8       /* A8-A15 */
#define PIN_AH8 15    /* PTC00 */
#define PIN_AH9 22    /* PTC01 */
#define PIN_AH10 23   /* PTC02 */
#define PIN_AH11 9    /* PTC03 */
#define PIN_AH12 10   /* PTC04 */
#define PIN_AH13 13   /* PTC05 */
#define PIN_AH14 11   /* PTC06 */
#define PIN_AH15 12   /* PTC07 */
#define PIN_AS 16     /* PTB00 */
#define PIN_RW 17     /* PTB01 */
#define PIN_IRQ 19    /* PTB02 */
#define PIN_XIRQ 18   /* PTB03 */
#define PIN_SCITXD 0  /* PTB16 */
#define PIN_SCIRXD 1  /* PTB17 */
#define PIN_EXTAL 29  /* PTB18 */
#define PIN_E 30      /* PTB19 */
#define PIN_MODA 33   /* PTE24 */
#define PIN_LIR 33    /* PTE24 */
#define PIN_MODB 34   /* PTE25 */
#define PIN_RESET 25  /* PTA05 */
#define PIN_USRSW 31  /* PTB10 */
#define PIN_USRLED 32 /* PTB11 */
#endif

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
