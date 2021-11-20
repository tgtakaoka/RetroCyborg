        cpu     6801
        include "mc6801.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $DF00
        include "mc6850.inc"

        org     $1000
stack:  equ     *-1             ; MC6801's SP is post-decrement/pre-increment

        org     $1000
initialize:
        lds     #stack
        ldaa    #CDS_RESET_gc   ; Master reset
        staa    ACIA_control
        ldaa    #WSB_8N1_gc     ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        staa    ACIA_control

receive_loop:
        ldaa    ACIA_status
        bita    #RDRF_bm
        beq     receive_loop
receive_data:
        ldab    ACIA_data
transmit_loop:
        ldaa    ACIA_status
        bita    #TDRE_bm
        beq     transmit_loop
transmit_data:
        stab    ACIA_data
        cmpb    #$0d
        bne     receive_loop
        ldab    #$0a
        bra     transmit_loop

        org     VEC_RESET
        fdb     initialize
