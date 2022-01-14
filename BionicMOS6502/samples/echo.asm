        cpu     6502
        include "mos6502.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $DF00
        include "mc6850.inc"

stack:  equ     $01ff

        org     $1000
initialize:
        ldx     #stack
        txs
        cld                     ; clear decimal flag
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #WSB_8N1_gc     ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        sta     ACIA_control

receive_loop:
        lda     ACIA_status
        and     #RDRF_bm
        beq     receive_loop
receive_data:
        lda     ACIA_data
        pha
transmit_loop:
        lda     ACIA_status
        and     #TDRE_bm
        beq     transmit_loop
transmit_data:
        pla
        sta     ACIA_data
        cmp     #$0d
        bne     receive_loop
        lda     #$0a
        pha
        bne     transmit_loop

        org     VEC_RESET
        fdb     initialize
