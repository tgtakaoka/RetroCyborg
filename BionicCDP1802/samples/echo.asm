        cpu     1802
        include "cdp1802.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     0DF00H
        include "mc6850.inc"

        org     0100H
main:
        ldi     ACIA >> 8
        phi     R8
        ldi     ACIA & 0ffh
        plo     R8              ; R8=ACIA
        ldi     CDS_RESET_gc    ; Master reset
        str     R8              ; ACIA_control
        ldi     WSB_8N1_gc      ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        str     R8              ; ACIA_control

receive_loop:
        ldn     R8              ; ACIA_status
        ani     RDRF_bm
        bz      receive_loop
receive_data:
        inc     R8              ; R8=ACIA_data
        ldn     R8              ; d=char
        dec     R8              ; R8=ACIA_status
        plo     R7              ; R7.0=char
transmit_loop:
        ldn     R8              ; ACIA_status
        ani     TDRE_bm
        bz      transmit_loop
transmit_data:
        glo     R7
        inc     R8              ; R8=ACIA_data
        str     R8              ; ACIA_data
        dec     R8              ; R8=ACIA_status
        xri     0dh
        bnz     receive_loop
        ldi     0ah
        plo     R7              ; R7.0=char
        br      transmit_loop

        org     ORG_RESET
        dis                     ; IE=0
        db      00h             ; X:P=0:0
        ldi     main >> 8
        phi     R3
        ldi     main & 0xff
        plo     R3
        sep     R3               ; jump to main with R3 as PC
