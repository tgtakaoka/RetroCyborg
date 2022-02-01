        cpu     6801
        include "mc6801.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $DF00
        include "mc6850.inc"

        org     $2000

rx_queue_size:  equ     128
rx_queue:       rmb     rx_queue_size
tx_queue_size:  equ     128
tx_queue:       rmb     tx_queue_size
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm
RX_INT_TX_INT:  equ     WSB_8N1_gc|RIEB_bm|TCB_EI_gc
tx_int_control: rmb     1

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
        ;; initialize ACIA
        ldaa    #CDS_RESET_gc   ; master reset
        staa    ACIA_control
        ldaa    #RX_INT_TX_NO
        staa    ACIA_control
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
        ldaa    #RX_INT_TX_INT  ; enable Tx interrupt
        staa    ACIA_control
        com     tx_int_control
putchar_exit:
        pula                    ; restore CC
        tap
        pula
        pulb
        pulx
        rts

        include "queue.inc"

isr_irq:
        ldab    ACIA_status
        bitb    #IRQF_bm
        beq     isr_irq_exit
        bitb    #FERR_bm|OVRN_bm|PERR_bm
        beq     isr_irq_receive
        ldaa    ACIA_data       ; reset error flags
isr_irq_receive:
        bitb    #RDRF_bm
        beq     isr_irq_send
        ldaa    ACIA_data       ; receive character
        ldx     #rx_queue
        jsr     queue_add
isr_irq_send:
        bitb    #TDRE_bm
        beq     isr_irq_exit
        ldx     #tx_queue
        jsr     queue_remove
        bcc     isr_irq_send_empty
        staa    ACIA_data       ; send character
isr_irq_exit:
        rti
isr_irq_send_empty:
        ldaa    #RX_INT_TX_NO
        staa    ACIA_control    ; disable Tx interrupt
        clr     tx_int_control
        rti

        org     VEC_IRQ1
        fdb     isr_irq

        org     VEC_RESET
        fdb     initialize
