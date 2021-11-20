        cpu     6801
        include "mc6801.inc"

;;; SCI: Emable Rx and Tx
RX_ON_TX_ON:   equ     TRCSR_TE_bm|TRCSR_RE_bm

        org     $1000
stack:  equ     *-1      ; MC6801's SP is post-decrement/pre-increment

        org     $1000
initialize:
        lds     #stack
	;; Initialize SCI
        ldaa    #CCFS_NRZ_gc|SS_DIV16_gc
        staa    RMCR            ; set NRZ and E/16
        ldaa    #RX_ON_TX_ON
        staa    TRCSR
        bra     receive_loop

receive_error:
        ldaa    SCRDR          ; Reset ORFE
receive_loop:
        ldaa    TRCSR
        bita    #TRCSR_ORFE_bm  ; Overrun or framing error?
        bne     receive_error
        bita    #TRCSR_RDRF_bm  ; Receive Data Register Full?
        beq     receive_loop    ; no
receive_data:
        ldab    SCRDR          ; Received data
transmit_loop:
        ldaa    TRCSR
        bita    #TRCSR_TDRE_bm  ; Transmit Data Register Empty?
        beq     transmit_loop   ; no
transmit_data:
        stab    SCTDR          ; Transmit data
        cmpb    #$0d
        bne     receive_loop
        ldab    #$0a
        bra     transmit_loop

        org     VEC_RESET
        fdb     initialize
