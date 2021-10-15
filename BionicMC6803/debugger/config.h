#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define MPU_NAME "MC6803"
#define MPU_MODE 2  // Non-Multiplexed Mode (Internal RAM enabled)

#define VERSION_TEXT "* Bionic" MPU_NAME " 0.1"
#define Console Serial

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
#define PORT_AH C     /* GPIOC */
#define AH_gp 0       /* PTC00 */
#define AH_gm 0xFF    /* PTC00-PTC07 */
#define PIN_AH8 15    /* PTC00 */
#define PIN_AH9 22    /* PTC01 */
#define PIN_AH10 23   /* PTC02 */
#define PIN_AH11 9    /* PTC03 */
#define PIN_AH12 10   /* PTC04 */
#define PIN_AH13 13   /* PTC05 */
#define PIN_AH14 11   /* PTC06 */
#define PIN_AH15 12   /* PTC07 */
#define PIN_PC0 3     /* PTA12 */
#define PIN_PC1 4     /* PTA13 */
#define PIN_PC2 26    /* PTA14 */
#define PIN_AS 28     /* PTA16 */
#define PIN_RW 39     /* PTA17 */
#define PIN_RESET 25  /* PTA05 */
#define PIN_IRQ1 31   /* PTB10 */
#define PIN_NMI 32    /* PTB11 */
#define PIN_CLOCK 29  /* PTB18 */
#define PIN_E 30      /* PTB19 */
#define PIN_SCITXD 0  /* PTB16 */
#define PIN_SCIRXD 1  /* PTB17 */
#define PIN_USRLED 18 /* PTB03 */
#define PIN_USRSW 19  /* PTB02 */
/**
 * For LILBUG's trace, Timer ouput (P21/PC1) is connected to #NMI on board.
 */

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
