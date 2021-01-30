/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
/*
 * Clock generator for HD6309E.
 * Supporting IO address detecting and STEP bus cycle.
 *
 * 1. When #STEP=H, oscillate CLK_E and CLK_Q about 1MHz.
 * 2. On the falling edge of CLK_Q, check whether address bus has IO
 *    address or not [v/V].
 * 3. If an IO access is found [V], assert #INT [I] and stretch
 *    CLK_Q=L and CLK_E=H until #ACK is asserted.
 * 4. When write bus cycle, valid address [V] and valid data [W] are
 *    on CPU bus.
 * 5. When read bus cycle, valid address [V] is on CPU bus, and
 *    debugger can put valid date on CPU bus [R].
 * 6. When #ACK is asserted [A], valid data [R] is read on the falling
 *    edge of CLK_E [e] then negate #INT [i].
 * 7. Stretch CLK_Q=L and CLK_E=L until #ACK is negated, and debugger
 *     can make bus as input [R->.].
 * 8. When #ACK is negated [a], start oscillating CLK_Q and CLK_E
 *    about 1MHz [Q/E].
 *          _____v      _____V                _____v
 * CLK_Q __|     |_____|     |____________Q__|     |_____
 *             _____       __________e           _____
 * CLK_E _____|     |_____|          |____E_____|     |__
 *  ADDR ...vvvvvvvv....VVVVVVVVVVVVV..........vvvvvvvv...
 *    WR ...wwwwwwww....WWWWWWWWWWWWW..........wwwwwwww...
 *    RD ..........rrr..............RRR..............rrr..
 *       ____________________I       i____________________
 *  #INT                     |_______|
 *       ___________________________A     a______________
 *  #ACK                            |_____|
 *
 *
 * 1. On the rising edge of CLK_E, check whether #STEP is asserted or
 *    not [s/S].
 * 2. When #STEP is asserted [S], assert #INT [I] and stretch CLK_Q=E
 *    and CLK_E=H until #STEP is negated.
 * 3. #HALT (or #RESET) can be asserted and recognized on the falling
 *    edge of CLK_Q [H], then CPU goes halt (or reset).
 * 4. When #STEP is negated [t], advance CLK_Q and CLK_E 1/4 phase
 *    [Q/E] then negate #INT [i].
 * 5. Stretch CLK_Q=L and CLK_E=L until #ACK is asserted, and debugger
 *    can capture valid data from CPU [W], memory [R], or debugger
 *    itself [D].
 * 6. When #ACK is asserted [A], valid data [R/D] is read on the
 *    falling edge of CLK_E [e], then wait for #ACK is negated.
 *    Debugger can re-enable #STEP if desired [S].
 * 7. When #ACK is negated [a], CLK_Q and CLK_E are back on
 *    oscillating about 1MHz.
 * 8. Instead of #ACK asserted, #STEP can also be negated [s] to start
 *    oscillating CLK_Q and CLK_E about 1MHz.
 *
 *       __s_____         t_______S          t____________s__________
 * #STEP         |_____S__|       |_______S__|
 *       _____       _____Q_H           _____Q_h           _____
 * CLK_Q      |_____|       |_i_A___a__|       |_i_A___a__|     |____
 *         s_____      S__E___i_A         S__E___i_A          _____
 * CLK_E __|     |_____|        |___a_____|        |___a_____|     |_
 *  ADDR aaaaaaaa....AAAAAAAAAAA........AAAAAAAAAAAAA....aaaaaaaa....
 *    WR wwwwwwww....WWWWWWWWWWW.........................wwwwwwww....
 *    RD .......rrr...............................RRR..........rrr...
 * Debuggr ....................DDD................DDD................
 *       ________________                    __h_____________________
 * #HALT                 |__H_______________|
 *       ______________I      i___________I      i___________________
 * #INT                |______|           |______|
 *       _______________________A   a______________A   a_____________
 * #ACK                         |___|              |___|
 */

#include "pins.h"
#include "pins_map.h"

#define OPTIMIZED_ASM 1

void setup() {
    noInterrupts();
    Pins.begin();  // QE=LL (0)
    Pins.nop();
}

void loop() {
    uint8_t io_adrh = IO_ADRH;
#ifdef IO_ADRM
    uint8_t io_adrm = IO_ADRM;
#endif
#ifdef IO_ADRL
    uint8_t io_adrl = IO_ADRL;
#endif

#if OPTIMIZED_ASM
    uint8_t tmp = 0;
    // clang-format off
    asm volatile(
    " rjmp L%=_entry"                "\n"
    "L%=_main_loop:"                 "\n" //     QE=3[5]
    " cbi  %[eclk_port],%[eclk_pin]" "\n" // [2] QE=0:LL
    " nop"                           "\n" // [1]
    " nop"                           "\n" // [1]
    " nop"                           "\n" // [1] QE=0[5]
    "L%=_check_step:"                "\n"
    " sbi  %[qclk_port],%[qclk_pin]" "\n" // [2] QE=1:HL
    " nop"                           "\n" // [1]
    " nop"                           "\n" // [1]
    " nop"                           "\n" // [1] QE=1[5]
    " sbi  %[eclk_port],%[eclk_pin]" "\n" // [2] QE=2:HH
    " sbis %[step_port],%[step_pin]" "\n" // [1:no,2:skip] STEP=H?
    " rjmp L%=_step_loop"            "\n" // [2] STEP=L, go step
    "L%=_check_io_addr:"             "\n"
    " in   %[tmp],%[adrh]"           "\n" // [1] QE=2[5]
    " cbi  %[qclk_port],%[qclk_pin]" "\n" // [2] QE=3:LH
#if BUS_gm(ADRH) != 0xFF
    " andi %[tmp],%[adrh_gm]"        "\n"
#endif
    " cpse %[tmp],%[io_adrh]"        "\n" // [1:no,2:skip] IO_ADRH?
    " rjmp L%=_main_loop"            "\n" // [2] no, not IO access
#ifdef IO_ADRM
    " in   %[tmp],%[adrm]"           "\n" // [1]
#if BUS_gm(ADRM) != 0xFF
    " andi %[tmp],%[adrm_gm]"        "\n" // [1]
#endif
    " cpse %[tmp],%[io_adrm]"        "\n" // [1:no,2:skip] IO_ADRM?
    " rjmp L%=_main_loop"            "\n" // [2] (QE=3[9])
#endif
#ifdef IO_ADRL
    " in   %[tmp],%[adrl]"           "\n" // [1]
#if BUS_gm(ADRL) != 0xFF
    " andi %[tmp],%[adrl_gm]"        "\n" // [1]
#endif
    " cpse %[tmp],%[io_adrl]"        "\n" // [1:no,2:skip] IO_ADRL?
    " rjmp L%=_main_loop"            "\n" // [2] (QE=3[9])
#endif
    " cbi  %[int_port],%[int_pin]"   "\n" // [2] INT=L
    " sbic %[ack_port],%[ack_pin]"   "\n" // [1:no,2:skip] ACK=L?
    " rjmp .-4"                      "\n" // [2] ACK=H, wait ACK=L
    " cbi  %[eclk_port],%[eclk_pin]" "\n" // [2] QE=0:LL
    " sbi  %[int_port],%[int_pin]"   "\n" // [2] INT=H
    " sbis %[ack_port],%[ack_pin]"   "\n" // [1:no,2:skip] ACK=H?
    " rjmp .-4"                      "\n" // [2] ACK=L, wait ACK=H
    "L%=_entry:"                     "\n" //     QE=0[6]
    " sbi  %[qclk_port],%[qclk_pin]" "\n" // [2] QE=1:HL
    " nop"                           "\n" // [1]
    " nop"                           "\n" // [1]
    " nop"                           "\n" // [1] QE=1:[5]
    " sbi  %[eclk_port],%[eclk_pin]" "\n" // [2] QE=2:HH
    " rjmp L%=_check_io_addr"        "\n" // [2]
    "L%=_step_loop:"                 "\n"
    " cbi  %[int_port],%[int_pin]"   "\n" // [2] INT=L
    " sbis %[step_port],%[step_pin]" "\n" // [1:no,2:skip] STEP=H?
    " rjmp .-4"                      "\n" // [2] STEP=L, wait STEP=H
    " cbi  %[qclk_port],%[qclk_pin]" "\n" // [2] QE=3:LH
    " sbi  %[int_port],%[int_pin]"   "\n" // [2] INT=H
    " sbic %[ack_port],%[ack_pin]"   "\n" // [1:no,2:skip] ACK=L?
    " rjmp .-4"                      "\n" // [2] ACK=H, wait ACK=L
    " cbi  %[eclk_port],%[eclk_pin]" "\n" // [2] QE=0:LL
    " sbis %[ack_port],%[ack_pin]"   "\n" // [1:no,2:skip] ACK=H?
    " rjmp .-4"                      "\n" // [2] ACK=L, wait ACK=H
    " rjmp L%=_check_step"           "\n" // [2]
    :
    : [tmp]       "d" (tmp),
      [io_adrh]   "r" (io_adrh),
      [adrh]      "I" (_SFR_IO_ADDR(PIN(ADRH))),
      [adrh_gm]   "M" (BUS_gm(ADRH)),
#ifdef IO_ADRM
      [io_adrm]   "r" (io_adrm),
      [adrm]      "I" (_SFR_IO_ADDR(PIN(ADRM))),
      [adrm_gm]   "M" (BUS_gm(ADRM)),
#endif
#ifdef IO_ADRL
      [io_adrl]   "r" (io_adrl),
      [adrl]      "I" (_SFR_IO_ADDR(PIN(ADRL))),
      [adrl_gm]   "M" (BUS_gm(ADRL)),
#endif
      [qclk_port] "I" (_SFR_IO_ADDR(POUT(CLK_Q))),
      [qclk_pin]  "I" (__PIN__(CLK_Q)),
      [eclk_port] "I" (_SFR_IO_ADDR(POUT(CLK_E))),
      [eclk_pin]  "I" (__PIN__(CLK_E)),
      [int_port]  "I" (_SFR_IO_ADDR(POUT(INT))),
      [int_pin]   "I" (__PIN__(INT)),
      [step_port] "I" (_SFR_IO_ADDR(PIN(STEP))),
      [step_pin]  "I" (__PIN__(STEP)),
      [ack_port]  "I" (_SFR_IO_ADDR(PIN(ACK))),
      [ack_pin]   "I" (__PIN__(ACK))
      );
// clang-formt on
#else
  goto entry;
  for (;;) {
    Pins.clrE();            // QE=0:LL
    Pins.nop();
  check_step:
    Pins.setQ();            // QE=1:HL
    Pins.nop();
    Pins.setE();            // QE=2:HH
    if (Pins.isStep())
      goto step_loop;
  check_io_addr:
    Pins.clrQ();            // QE=3:LH
    if (!Pins.isIoAddr())
      continue;
    Pins.assertInt();
    while (!Pins.isAck())
      ;
    Pins.clrE();            // QE=0:LL
    Pins.negateInt();
    while (Pins.isAck())
      ;
  entry:
    Pins.setQ();            // QE=1:HL
    Pins.nop();
    Pins.setE();            // QE=2:HH
    goto check_io_addr;

  step_loop:
    Pins.assertInt();
    while (Pins.isStep())
      ;
    Pins.clrQ();            // QE=3:LH
    Pins.negateInt();
    while (!Pins.isAck())
      ;
    Pins.clrE();            // QE=0:LL
    while (Pins.isAck())
      ;
    goto check_step;
  }
#endif
}
