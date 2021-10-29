#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define MPU_NAME "MC146805"

#define VERSION_TEXT "* Bionic" MPU_NAME " 0.1"
#define Console Serial

#define IO_BASE_ADDR 0x1F00

#define PORT_DB D     /* GPIOD */
#define DB_gp 0       /* PTD00 */
#define DB_gm 0xFF    /* PTD00-PTD07 */
#define PIN_B0 2      /* PTD00 */
#define PIN_B1 14     /* PTD01 */
#define PIN_B2 7      /* PTD02 */
#define PIN_B3 8      /* PTD03 */
#define PIN_B4 6      /* PTD04 */
#define PIN_B5 20     /* PTD05 */
#define PIN_B6 21     /* PTD06 */
#define PIN_B7 5      /* PTD07 */
#define PORT_AH C     /* GPIOC */
#define AH_gp 0       /* PTC00 */
#define AH_gm 0x1F    /* PTC00-PTC04 */
#define PIN_AH8 15    /* PTC00 */
#define PIN_AH9 22    /* PTC01 */
#define PIN_AH10 23   /* PTC02 */
#define PIN_AH11 9    /* PTC03 */
#define PIN_AH12 10   /* PTC04 */
#define PIN_DS 33     /* PTE24 */
#define PIN_AS 28     /* PTA16 */
#define PIN_RW 39     /* PTA17 */
#define PIN_LI 34     /* PTE25 */
#define PIN_RESET 25  /* PTA05 */
#define PIN_IRQ 31    /* PTB10 */
#define PIN_CLOCK 29  /* PTB18 */
#define PIN_USRLED 18 /* PTB03 */
#define PIN_USRSW 19  /* PTB02 */

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
