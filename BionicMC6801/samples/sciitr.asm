        cpu     6801
        include "mc6801.inc"

        org     $2000

rx_queue_size:  equ     128
rx_queue:       rmb     rx_queue_size
tx_queue_size:  equ     128
tx_queue:       rmb     tx_queue_size
tx_int_control: rmb     1

;;; SCI: Enable Rx interrupt
RX_INT_TX_NO:   equ     TRCSR_TE_bm|TRCSR_RE_bm|TRCSR_RIE_bm
RX_INT_TX_INT:  equ     RX_INT_TX_NO|TRCSR_TIE_bm

        org     $1000
stack:  equ     *-1             ; MC6801's SP is post-decrement/pre-increment

        org     $1000
initialize:
        lds     #stack
        ldx     #rx_queue
        ldab    #rx_queue_size
        jsr     queue_init
        ldx     #tx_queue
        ldab    #tx_queue_size
        jsr     queue_init
        ;; Initialize SCI
        ldaa    #CCFS_NRZ_gc|SS_DIV16_gc
        staa    RMCR            ; set NRZ and E/16
        ldaa    #RX_INT_TX_NO
        staa    TRCSR           ; Enable Tx and Rx/Interrupt
        clr     tx_int_control  ; disable Tx interrupt
        cli                     ; enable IRQ

receive_loop:
        bsr     getchar
        bcc     receive_loop
echo_back:
        tab
        bsr     putchar         ; echo
        ldaa    #' '            ; space
        bsr     putchar
        bsr     put_hex8        ; print in hex
        ldaa    #' '            ; space
        bsr     putchar
        bsr     put_bin8        ; print in binary
        bsr     newline
        bra     receive_loop

;;; Put newline
;;; @clobber A
newline:
        ldaa    #$0d
        bsr     putchar
        ldaa    #$0a
        bra     putchar

;;; Print uint8_t in hex
;;; @param B uint8_t value to be printed in hex.
;;; @clobber A
put_hex8:
        ldaa    #'0'
        bsr     putchar
        ldaa    #'x'
        bsr    putchar
        tba
        lsra
        lsra
        lsra
        lsra
        bsr     put_hex4
        tba
put_hex4:
        anda    #$0f
        cmpa    #10
        blo     put_hex8_dec
        adda    #'A'-10
        bra     putchar
put_hex8_dec:
        adda    #'0'
        bra    putchar

;;; Print uint8_t in binary
;;; @param B uint8_t value to be printed in binary.
;;; @clobber A
put_bin8:
        pshb
        ldaa    #'0'
        bsr     putchar
        ldaa    #'b'
        bsr     putchar
        bsr     put_bin4
        lslb
        bsr     put_bin4
        pulb
        rts
put_bin4:
        bsr     put_bin2
        lslb
put_bin2:
        bsr     put_bin1
        lslb
put_bin1:
        ldaa    #'0'
        tstb                    ; chech MSB
        bpl     put_bin0        ; MSB=0
        inca                    ; MSB=1
put_bin0:
        bra     putchar

;;; Get character
;;; @return A
;;; @return CC.C 0 if no character
getchar:
        pshx
        pshb
        tpa
        psha                    ; save CC
        sei                     ; disable IRQ
        ldx     #rx_queue
        jsr     queue_remove
        tab                     ; char? in B
        pula                    ; restore CC to A
        bcs     getchar_exit
        tap
        clc                     ; clear carry
        pulb
        pulx
        rts
getchar_exit:
        tap
        sec                     ; set carry
        tba
        pulb
        pulx
        rts

;;; Put character
;;; @param A
putchar:
        pshx
        pshb
        psha
        tab                     ; char in B
        tpa
        psha                    ; save CC
putchar_retry:
        tba                     ; char in A
        ldx     #tx_queue
        sei                     ; disable IRQ
        jsr     queue_add
        cli                     ; enable IRQ
        bcc     putchar_retry   ; branch if queue is full
        sei                     ; disable IRQ
        tst     tx_int_control
        bne     putchar_exit
        ldaa    #RX_INT_TX_INT  ; Enable Tx interrupt
        staa    TRCSR
        com     tx_int_control
putchar_exit:
        pula                    ; restore CC
        tap
        pula
        pulb
        pulx
        rts

        include "queue.inc"

isr_sci:
        ldab    TRCSR
        bitb    #TRCSR_ORFE_bm
        beq     isr_sci_receive
        ldaa    SCRDR           ; reset ORFE
isr_sci_receive:
        bitb    #TRCSR_RDRF_bm
        beq     isr_sci_send
        ldaa    SCRDR           ; receive character
        ldx     #rx_queue
        jsr     queue_add
isr_sci_send:
        bitb    #TRCSR_TDRE_bm
        beq     isr_sci_exit
        ldx     #tx_queue
        jsr     queue_remove
        bcc     isr_sci_send_empty
        staa    SCTDR           ; send character
isr_sci_exit:
        rti
isr_sci_send_empty:
        ldaa    #RX_INT_TX_NO
        staa    TRCSR           ; disable Tx interrupt
        clr     tx_int_control
        rti

        org     VEC_SCI
        fdb     isr_sci

        org     VEC_RESET
        fdb     initialize
