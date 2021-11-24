        cpu     6811
        include "mc68hc11d.inc"

;;; SCI: Emable Rx and Tx
RX_ON_TX_ON:   equ     SCCR2_TE_bm|SCCR2_RE_bm

        org     $1000
stack:  equ     *-1      ; MC6801's SP is post-decrement/pre-increment

        org     $1000
device_base:
        dc.w    $0000   ; device base
initialize:
        lds     #stack
	;; Initialize SCI
        ldx     device_base
        clr     SCCR1,x                    ; 8bit 1stop
        ldaa    #BAUD_SCP1_gc|BAUD_SCR1_gc ; E/16
        staa    BAUD,x
        ldaa    #RX_ON_TX_ON
        staa    SCCR2,x
        bra     receive_loop

receive_error:
        ldaa    SCDR,x          ; Reset OR/NF/FE
receive_loop:
;;; Overrun or noise or framing error?
        brset   SCSR,x, #SCSR_OR_bm|SCSR_NF_bm|SCSR_FE_bm, receive_error
;;; Receive Data Register Full?
        brclr   SCSR,x, #SCSR_RDRF_bm, receive_loop
        beq     receive_loop    ; no
receive_data:
        ldab    SCDR,x          ; Received data
transmit_loop:
;;; Transmit Data Register Empty?
        brclr   SCSR,x, #SCSR_TDRE_bm, transmit_loop
transmit_data:
        stab    SCDR,x          ; transmit data
        cmpb    #$0d
        bne     receive_loop
        ldab    #$0a
        bra     transmit_loop

        org     VEC_RESET
        fdb     initialize
