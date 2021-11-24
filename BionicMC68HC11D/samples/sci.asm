        cpu     6811
        include "mc68hc11d.inc"

;;; SCI: Emable Rx and Tx
RX_ON_TX_ON:   equ     SCCR2_TE_bm|SCCR2_RE_bm

        org     $1000
stack:  equ     *-1      ; MC6801's SP is post-decrement/pre-increment

        org     $1000
initialize:
        lds     #stack
	;; Initialize SCI
        clr     SCCR1           ; 8bit 1stop
        ldaa    #BAUD_SCP1_gc|BAUD_SCR1_gc ; E/16
        staa    BAUD
        ldaa    #RX_ON_TX_ON
        staa    SCCR2
        bra     receive_loop

receive_error:
        ldaa    SCDR            ; Reset OR/NF/FE
receive_loop:
        ldaa    SCSR
        bita    #SCSR_OR_bm|SCSR_NF_bm|SCSR_FE_bm ; Overrun or noise or framing error?
        bne     receive_error
        bita    #SCSR_RDRF_bm   ; Receive Data Register Full?
        beq     receive_loop    ; no
receive_data:
        ldab    SCDR            ; Received data
transmit_loop:
        ldaa    SCSR
        bita    #SCSR_TDRE_bm   ; Transmit Data Register Empty?
        beq     transmit_loop   ; no
transmit_data:
        stab    SCDR            ; transmit data
        cmpb    #$0d
        bne     receive_loop
        ldab    #$0a
        bra     transmit_loop

        org     VEC_RESET
        fdb     initialize
