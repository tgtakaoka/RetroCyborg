/*
 * Clock generator for HD6309E.
 *
 * If isIoAddr() when CLK_E=H & CLK_Q=L
 *  Assert INT=L
 *  (We can drive/read Data Bus here)
 *  Strech CLK_E=H & CLK_Q=L until ACK=L
 *  (We have to relase Data Bus here)
 *  Negate INT=H and CLK_E=L until ACK=H
 *
 * If STEP=L when CLK_E=L & CLK_Q=L
 *  Advance clock CLK_Q=H, CLK_E=H, CLK_Q=L
 *  Strech CLK_E=H & CLK_Q=L until ACK=L
 *  (We can drive/read Data Bus here)
 *  Advance clock CLK_E=L
 *  Strech CLK_E=L & CLK_Q=L until ACK=H
 *  (We have to relase Data Bus here)
 */

#include "pins.h"

void setup() {
  cli();
  Pins.begin();
}

void loop() {
  do {
    Pins.clrE();

    while (Pins.isStep()) {
      Pins.setQ();
      Pins.setE();
      Pins.clrQ();
      while (!Pins.isAck() && Pins.isStep())
        ;
      Pins.clrE();
      while (Pins.isAck() && Pins.isStep())
        ;
    }

    Pins.setQ();
    Pins.setE();
    Pins.clrQ();
  } while (!Pins.isIoAddr());

  Pins.assertInt();
  while (!Pins.isAck())
    ;

  Pins.negateInt();
  Pins.clrE();
  while (Pins.isAck())
    ;
}
