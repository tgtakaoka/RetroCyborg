        cpu     6805
        include "mc6805.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $1F00
        include "mc6850.inc"

        org     $20
save_a: rmb     1

        org     $0100
initialize:
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #WSB_8N1_gc     ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        sta     ACIA_control

loop:   bsr     getchar
echo:   bsr     putchar
        cmp     #$0D            ; Carriage Return
        bne     loop
        lda     #$0A            ; Newline
        bra     echo

getchar:
        lda     ACIA_status
        bit     #RDRF_bm
        beq     getchar
receive_data:
        lda     ACIA_data
        rts

putchar:
        sta     save_a
transmit_loop:
        lda     ACIA_status
        bit     #TDRE_bm
        beq     transmit_loop
transmit_data:
        lda     save_a
        sta     ACIA_data
        rts

        org     VEC_RESET
        fdb     initialize
