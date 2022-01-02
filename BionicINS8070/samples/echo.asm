        cpu     ins8070
        include "ins8070.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     0xDF00
        include "mc6850.inc"
ACIA_C: equ     0               ; ACIA control offset
ACIA_S: equ     0               ; ACIA status offset
ACIA_D: equ     1               ; ACIA data register offset

        org     0x1000
stack:
initialize:
        ld      sp, =stack
        ld      p2, =ACIA
        ld      a, =CDS_RESET_gc        ; Master reset
        st      a, ACIA_C, p2
        ld      a, =WSB_8N1_gc  ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        st      a, ACIA_C, p2

receive_loop:
        ld      a, ACIA_S, p2
        and     a, =RDRF_bm
        bz      receive_loop
receive_data:
        ld      a, ACIA_D, p2
        xch     a, e            ; E=data
transmit_loop:
        ld      a, ACIA_S, p2
        and     a, =TDRE_bm
        bz      transmit_loop
transmit_data:
        xch     a, e            ; A=data
        st      a, ACIA_D, p2
        xor     a, =0x0d
        bnz     receive_loop
        ld      a, =0x0a
        xch     a, e            ; E=data
        bra     transmit_loop

        org     ORG_RESTART
        jmp     initialize
