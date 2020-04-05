        cpu     6809
        include "mc6809.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $FFC0
        include "mc6850.inc"

        org     $F000
stack:  equ     *

        org     $1000
initialize:
        lds     #stack
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #WSB_8N1_gc        ; 8 bits + No Parity + 1 Stop Bits
        ora     #TCB_EI_gc|RIEB_bm ; Transmit, Receive interrupts enable
        sta     ACIA_control
        orcc    #CC_IRQ         ; Set Interrupt mask

receive_loop:
        lda     ACIA_status
        bita    #RDRF_bm
        beq     receive_loop
receive_data:
        ldb     ACIA_data
transmit_loop:
        lda     ACIA_status
        bita    #TDRE_bm
        beq     transmit_loop
transmit_data:
        stb     ACIA_data
        cmpb    #$0d
        bne     receive_loop
        ldb     #$0a
        bra     transmit_loop

        org     $FFFE
        fdb     initialize
