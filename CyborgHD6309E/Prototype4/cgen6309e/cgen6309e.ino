/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
/*
 * Clock generator for HD6309E.
 * Supporting IO address detecting and STEP bus cycle.
 *
 * [IO address detecting]
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
 *          Q_____v     Q_____V               Q_____v
 * CLK_Q ___|     |_____|     |____________a__|     |_____ CLK_Q
 *       e      _____e      __________e           _____e
 * CLK_E |_____|     |_____|          |____a_____|     |__ CLK_E
 *       _____________________I        i__________________
 *  #INT                      |________|                   #INT
 *       ____________________________A     a______________
 *  #ACK                             |_____|               #ACK
 *         _________   ______________   ______________   _
 *  ADDR e<_______v_>e<_______V______>e<____________v_>e<_ ADDR
 *            ______      ___________           ______
 * DB/WR ---Q<______>e--Q<WWWWWWWWWWW>e-------Q<______>e-- DB/WR
 *                 _                _                _
 * DB/RD ---------<r>e-------------<R>e-------------<r>e-- DB/RD
 *
 * [STEP bus cycle]
 * 1. On the rising edge of CLK_E, check whether #STEP is asserted or
 *    not [S].
 * 2. When #STEP is asserted [S], assert #INT [I] and stretch CLK_Q=H
 *    and CLK_E=H until #STEP is negated [s].
 * 3. While #STEP is asserted, #HALT can be asserted and recognized on
 *    the falling edge of CLK_Q [H], then CPU goes halt on the next
 *    cycle.
 * 4. When #STEP is negated [s], advance CLK_Q=L and CLK_E=H [Q/E]
 *    then negate #INT [i].
 * 5. Stretch CLK_Q=L and CLK_E=H until #ACK is asserted [A], and
 *    debugger can capture valid data from CPU or memory.
 * 6. When #ACK is asserted [A], valid data from memory or debugger is
 *    read on the falling edge of CLK_E [e], then wait for #ACK is
 *    negated [a].
 * 7. When #ACK is negated [a], CLK_Q and CLK_E are back on
 *    oscillating about 1MHz.
 *            Q_________s_H               Q_____H
 * CLK_Q _____|           |_i___A____a____|     |_____ CLK_Q
 *       _e      S__E___s_______Ae           S_E___e
 * CLK_E  |______|               |___a_______|     |__ CLK_E
 *       _______        s____________________s________
 * #STEP      |||SI_____|            |||_____S________ #STEP
 *       _________I         i_________________________
 * #INT           |_________|                          #INT
 *       _______________________A    a________________
 * #ACK                         |____|                 #ACK
 *       _____________                __________h_____
 * #HALT            |||_s_H__________|||________H_____ #HALT
 *          ____________________   _______________   _
 * BA/BS >e<____________________>e<_______________>e<_ BA/BS
 *       __________   ________________________   _____
 *   LIC __________>E<________________________>E<_____ AVMA
 */

#include "pins.h"
#include "pins_map.h"

#define OPTIMIZED_ASM 1

#if OPTIMIZED_ASM
#include "pins_base.h"
#endif

void setup() {
    noInterrupts();
    Pins.begin();  // QE=LL (0)
    Pins.nop();
}

void loop() {
#ifdef IO_ADRH
    uint8_t io_adrh = IO_ADRH;
#endif
#ifdef IO_ADRM
    uint8_t io_adrm = IO_ADRM;
#endif
#ifdef IO_ADRL
    uint8_t io_adrl = IO_ADRL;
#endif

#if OPTIMIZED_ASM
    // clang-format off
    uint8_t tmp = 0;
    asm volatile(
            " rjmp L%=_check_step"            "\n"

#ifdef IOR_PIN
            "L%=_check_step:"                 "\n"
            " sbi  %[qclk_port],%[qclk_pin]"  "\n"  // [1] QE=1[1](HL)
            " nop"                            "\n"  // [1] QE=1[2]
            " nop"                            "\n"  // [1] QE=1[3]

            " sbi  %[eclk_port],%[eclk_pin]"  "\n"  // [1] QE=2[1](HH)
            " sbis %[step_port],%[step_pin]"  "\n"  // [2] QE=2[3] STEP=H?
            " rjmp L%=_step_loop"             "\n"  // [1+2] STEP=L, go step

            " cbi  %[qclk_port],%[qclk_pin]"  "\n"  // [1] QE=3[1](LH)
            " sbis %[ior_port],%[ior_pin]"    "\n"  // [2] QE=3[3] IOR=H?
            " rjmp L%=_io_access"             "\n"  // [1+2] QE=3[4], IO access

            "L%=_main_loop:"                  "\n"
            " cbi  %[eclk_port],%[eclk_pin]"  "\n"  // [1] QE=0[1](LL)
            " rjmp L%=_check_step"            "\n"  // [2] QE=0[3]

#elif defined(IO_ADRH)

#if BUS_gm(ADRH) == 0xFF
            "L%=_main_loop:"                  "\n"
            " cbi  %[eclk_port],%[eclk_pin]"  "\n"  // [1] QE=0[1](LL)
            " nop"                            "\n"  // [1] QE=0[2]
            " nop"                            "\n"  // [1] QE=0[3]
            " nop"                            "\n"  // [1] QE=0[4]

            "L%=_check_step:"                 "\n"
            " sbi  %[qclk_port],%[qclk_pin]"  "\n"  // [1] QE=1[1](HL)
            " nop"                            "\n"  // [1] QE=1[2]
            " nop"                            "\n"  // [1] QE=1[3]
            " nop"                            "\n"  // [1] QE=1[4]

            " sbi  %[eclk_port],%[eclk_pin]"  "\n"  // [1] QE=2[1](HH)
            " sbis %[step_port],%[step_pin]"  "\n"  // [2] QE=2[3] STEP=H?
            " rjmp L%=_step_loop"             "\n"  // [1+2] QE=2[4] STEP=L, go step
            " in   %[tmp],%[adrh]"            "\n"  // [1] QE=2[4]

            " cbi  %[qclk_port],%[qclk_pin]"  "\n"  // [1] QE=3[1](LH)
            " cpse %[tmp],%[io_adrh]"         "\n"  // [2] QE=3[3] IO_ADRH?
            " rjmp L%=_main_loop"             "\n"  // [1+2] QE=3[4], not IO access

#else   // BUS_gm(ADRH) != 0xFF
            "L%=_main_loop:"                  "\n"
            " cbi  %[eclk_port],%[eclk_pin]"  "\n"  // [1] QE=0[1](LL)
            " nop"                            "\n"  // [1] QE=0[2]
            " nop"                            "\n"  // [1] QE=0[3]
            " nop"                            "\n"  // [1] QE=0[4]
            " nop"                            "\n"  // [1] QE=0[4]

            "L%=_check_step:"                 "\n"
            " sbi  %[qclk_port],%[qclk_pin]"  "\n"  // [1] QE=1[1](HL)
            " nop"                            "\n"  // [1] QE=1[2]
            " nop"                            "\n"  // [1] QE=1[3]
            " nop"                            "\n"  // [1] QE=1[4]
            " nop"                            "\n"  // [1] QE=1[5]

            " sbi  %[eclk_port],%[eclk_pin]"  "\n"  // [1] QE=2[1](HH)
            " sbis %[step_port],%[step_pin]"  "\n"  // [2] QE=2[3] STEP=H?
            " rjmp L%=_step_loop"             "\n"  // [1+2] QE=2[4] STEP=L, go step
            " nop"                            "\n"  // [1] QE=2[4]
            " in   %[tmp],%[adrh]"            "\n"  // [1] QE=2[5]

            " cbi  %[qclk_port],%[qclk_pin]"  "\n"  // [1] QE=3[1](LH)
            " andi %[tmp],%[adrh_gm]"         "\n"  // [1] QE=3[2]
            " cpse %[tmp],%[io_adrh]"         "\n"  // [2] QE=3[4] IO_ADRH?
            " rjmp L%=_main_loop"             "\n"  // [1+2] QE=3[5], not IO access
#endif  // BUS_gm(ADRH) == 0xFF

#ifdef IO_ADRM
            " in   %[tmp],%[adrm]"            "\n"  // [1] QE=3[5]
#if BUS_gm(ADRM) != 0xFF
            " andi %[tmp],%[adrm_gm]"         "\n"  // [1] QE=3[6]
#endif
            " cpse %[tmp],%[io_adrm]"         "\n"  // [2] QE=3[7/8] IO_ADRM?
            " rjmp L%=_main_loop"             "\n"  // [1+2] QE=3[7/8], not IO access
#endif
#ifdef IO_ADRL
            " in   %[tmp],%[adrl]"            "\n"  // [1] QE=3[9/10]
#if BUS_gm(ADRL) != 0xFF
            " andi %[tmp],%[adrl_gm]"         "\n"  // [1] QE=3[10/11]
#endif
            " cpse %[tmp],%[io_adrl]"         "\n"  // [2] QE=3[12/13] IO_ADRL?
            " rjmp L%=_main_loop"             "\n"  // [2] QE=3[13]
#endif
                  // Fall through to io_access

#else
#error "Unknown IO address detection method"
#endif

            // IO Address detected.
            "L%=_io_access:"                  "\n"
            " cbi  %[int_port],%[int_pin]"    "\n"  // [1] INT=L
            "L%=_wait_ack_low:"               "\n"
            " sbic %[ack_port],%[ack_pin]"    "\n"  // [2] ACK=L?
            " rjmp L%=_wait_ack_low"          "\n"  // [1+2] ACK=H, wait ACK=L
            " cbi  %[eclk_port],%[eclk_pin]"  "\n"  // [1] QE=LL(0)
            " sbi  %[int_port],%[int_pin]"    "\n"  // [1] INT=H
            "L%=_wait_ack_high:"              "\n"
            " sbis %[ack_port],%[ack_pin]"    "\n"  // [2] ACK=H?
            " rjmp L%=_wait_ack_high"         "\n"  // [1+2] ACK=L, wait ACK=H
            " rjmp L%=_check_step"            "\n"  // [2]

            // STEP requested.
            "L%=_step_loop:"                  "\n"
            " cbi  %[int_port],%[int_pin]"    "\n"  // [1] INT=L
            "L%=_wait_step_high:"             "\n"
            " sbis %[step_port],%[step_pin]"  "\n"  // [2] STEP=H?
            " rjmp L%=_wait_step_high"        "\n"  // [1+2] STEP=L, wait STEP=H
            " cbi  %[qclk_port],%[qclk_pin]"  "\n"  // [1] QE=LH(3)
            " sbi  %[int_port],%[int_pin]"    "\n"  // [1] INT=H
            "L%=_wait_ack_low2:"              "\n"
            " sbic %[ack_port],%[ack_pin]"    "\n"  // [2] ACK=L?
            " rjmp L%=_wait_ack_low2"         "\n"  // [1+2] ACK=H, wait ACK=L
            " cbi  %[eclk_port],%[eclk_pin]"  "\n"  // [1] QE=LL(0)
            "L%=_wait_ack_high2:"             "\n"
            " sbis %[ack_port],%[ack_pin]"    "\n"  // [2] ACK=H?
            " rjmp L%=_wait_ack_high2"        "\n"  // [1+2] ACK=L, wait ACK=H
            " rjmp L%=_check_step"            "\n"  // [2]
            :
            : [ tmp ] "d"(tmp),
#ifdef IOR_PIN
            [ ior_port ] "I"(_SFR_IO_ADDR(PIN(IOR))),
            [ ior_pin ] "I"(__PIN__(IOR)),
#endif
#ifdef IO_ADRH
            [ io_adrh ] "r"(io_adrh),
            [ adrh ] "I"(_SFR_IO_ADDR(PIN(ADRH))),
            [ adrh_gm ] "M"(BUS_gm(ADRH)),
#endif
#ifdef IO_ADRM
            [ io_adrm ] "r"(io_adrm),
            [ adrm ] "I"(_SFR_IO_ADDR(PIN(ADRM))),
            [ adrm_gm ] "M"(BUS_gm(ADRM)),
#endif
#ifdef IO_ADRL
            [ io_adrl ] "r"(io_adrl),
            [ adrl ] "I"(_SFR_IO_ADDR(PIN(ADRL))),
            [ adrl_gm ] "M"(BUS_gm(ADRL)),
#endif
            [ qclk_port ] "I"(_SFR_IO_ADDR(POUT(CLK_Q))),
            [ qclk_pin ] "I"(__PIN__(CLK_Q)),
            [ eclk_port ] "I"(_SFR_IO_ADDR(POUT(CLK_E))),
            [ eclk_pin ] "I"(__PIN__(CLK_E)),
            [ int_port ] "I"(_SFR_IO_ADDR(POUT(INT))),
            [ int_pin ] "I"(__PIN__(INT)),
            [ step_port ] "I"(_SFR_IO_ADDR(PIN(STEP))),
            [ step_pin ] "I"(__PIN__(STEP)),
            [ ack_port ] "I"(_SFR_IO_ADDR(PIN(ACK))),
            [ ack_pin ] "I"(__PIN__(ACK)));
// clang-format on
#else
    goto check_step;
    for (;;) {
    main_loop:
        Pins.clrE();  // QE=0(LL)
        Pins.nop();

    check_step:
        Pins.setQ();  // QE=1(HL)
        Pins.nop();
        Pins.setE();  // QE=2(HH)
        if (Pins.isStep())
            goto step_loop;
        Pins.clrQ();  // QE=3(LH)
        if (!Pins.isIoAddr())
            goto main_loop;

        // IO address detected.
    io_address:
        Pins.assertInt();
        while (!Pins.isAck())
            ;
        Pins.clrE();  // QE=0(LL)
        Pins.negateInt();
        while (Pins.isAck())
            ;
        goto check_step;

        // STEP requested.
    step_loop:
        Pins.assertInt();
        while (Pins.isStep())
            ;
        Pins.clrQ();  // QE=3(LH)
        Pins.negateInt();
        while (!Pins.isAck())
            ;
        Pins.clrE();  // QE=0(LL)
        while (Pins.isAck())
            ;
        goto check_step;
    }
#endif
}
