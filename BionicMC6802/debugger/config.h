#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define MPU_NAME "MC6802"
#define MPU_MODE 2  // Non-Multiplexed Mode (Internal RAM enabled)

#define VERSION_TEXT "* Bionic" MPU_NAME " 0.1"
#define Console Serial

#define IO_BASE_ADDR 0xDF00

#define PORT_DB D     /* GPIOD */
#define DB_gp 0       /* PTD00 */
#define DB_gm 0xFF    /* PTD00-PTD07 */
#define PIN_DB0 2     /* PTD00 */
#define PIN_DB1 14    /* PTD01 */
#define PIN_DB2 7     /* PTD02 */
#define PIN_DB3 8     /* PTD03 */
#define PIN_DB4 6     /* PTD04 */
#define PIN_DB5 20    /* PTD05 */
#define PIN_DB6 21    /* PTD06 */
#define PIN_DB7 5     /* PTD07 */
#define PORT_AL C     /* GPIOC */
#define AL_gp 0       /* PTC00 */
#define AL_gm 0xFFF   /* PTC00-PTC11 */
#define PIN_AL0 15    /* PTC00 */
#define PIN_AL1 22    /* PTC01 */
#define PIN_AL2 23    /* PTC02 */
#define PIN_AL3 9     /* PTC03 */
#define PIN_AL4 10    /* PTC04 */
#define PIN_AL5 13    /* PTC05 */
#define PIN_AL6 11    /* PTC06 */
#define PIN_AL7 12    /* PTC07 */
#define PIN_AL8 35    /* PTC08 */
#define PIN_AL9 36    /* PTC09 */
#define PIN_AL10 37   /* PTC10 */
#define PIN_AL11 38   /* PTC11 */
#define PORT_AH A     /* GPIOA */
#define AH_gp 0       /* PTA12 */
#define AH_gm 0xF000  /* PTA12-PTA15 */
#define PIN_AH12 3    /* PTA12 */
#define PIN_AH13 4    /* PTA13 */
#define PIN_AH14 26   /* PTA14 */
#define PIN_AH15 27   /* PTA15 */
#define PIN_HALT 28   /* PTA16 */
#define PIN_RW 39     /* PTA17 */
#define PIN_RESET 25  /* PTA05 */
#define PIN_VMA 17    /* PTB01 */
#define PIN_IRQ 31    /* PTB10 */
#define PIN_NMI 32    /* PTB11 */
#define PIN_CLOCK 29  /* PTB18 */
#define PIN_E 30      /* PTB19 */
#define PIN_BA 33     /* PTE24 */
#define PIN_MR 24     /* PTE26 */
#define PIN_USRLED 18 /* PTB03 */
#define PIN_USRSW 19  /* PTB02 */

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
