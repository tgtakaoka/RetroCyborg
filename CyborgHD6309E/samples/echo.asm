        cpu     6809
        include "mc6809.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $DF00
        include "mc6850.inc"

        org     $1000
stack:  equ     *

        org     $1000
initialize:
        lds     #stack
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #WSB_8N1_gc     ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        sta     ACIA_control

receive_loop:
        lda     ACIA_status
        lsra
        nop
        bcc     receive_loop
receive_data:
        ldb     ACIA_data
transmit_loop:
        lda     ACIA_status
        lsra
        lsra
        bcc     transmit_loop
transmit_data:
        stb     ACIA_data
        cmpb    #$0d
        bne     receive_loop
        ldb     #$0a
        bra     transmit_loop

        org     $FFFE
        fdb     initialize
