        cpu     6811
        include "mc68hc11d.inc"

        org     $2000

rx_queue_size:  equ     128
rx_queue:       rmb     rx_queue_size
tx_queue_size:  equ     128
tx_queue:       rmb     tx_queue_size
tx_int_control: rmb     1

;;; SCI: Enable Rx interrupt
RX_INT_TX_NO:   equ     SCCR2_TE_bm|SCCR2_RE_bm|SCCR2_RIE_bm
RX_INT_TX_INT:  equ     RX_INT_TX_NO|SCCR2_TIE_bm

        org     $1000
stack:  equ     *-1             ; MC6801's SP is post-decrement/pre-increment

        org     $1000
device_base:
        dc.w    $0000           ; device base
initialize:
        lds     #stack
        ldx     #rx_queue
        ldab    #rx_queue_size
        jsr     queue_init
        ldx     #tx_queue
        ldab    #tx_queue_size
        jsr     queue_init
        ;; Initialize SCI
        ldy     device_base
        clr     SCCR1,y                    ; 8bit 1stop
        ldaa    #BAUD_SCP1_gc|BAUD_SCR1_gc ; E/16
        staa    BAUD,y
        ldaa    #RX_INT_TX_NO
        staa    SCCR2,y         ; enable Tx and Rx/Interrupt
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
        staa    SCCR2
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
        ldy     device_base
        brclr   SCSR,y, #SCSR_OR_bm|SCSR_NF_bm|SCSR_FE_bm, isr_sci_receive
        ldaa    SCDR,y          ; reset ORFE
isr_sci_receive:
        brclr   SCSR,y, #SCSR_RDRF_bm, isr_sci_send
        ldaa    SCDR,y          ; receive character
        ldx     #rx_queue
        jsr     queue_add
isr_sci_send:
        brclr   SCSR,y, #SCSR_TDRE_bm, isr_sci_exit
        ldx     #tx_queue
        jsr     queue_remove
        bcc     isr_sci_empty
        staa    SCDR,y          ; send character
isr_sci_exit:
        rti
isr_sci_empty:
        ldaa    #RX_INT_TX_NO
        staa    SCCR2,y         ; disable Tx interrupt
        clr     tx_int_control
        rti

        org     VEC_SCI
        fdb     isr_sci

        org     VEC_RESET
        fdb     initialize
