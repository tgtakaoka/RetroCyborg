        cpu     6811
        include "mc68hc11d.inc"

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
        pshb
        tpa
        psha                    ; save CC
        sei                     ; disable IRQ
        pshx
        ldx     #rx_queue
        jsr     queue_remove
        pulx
        tab                     ; char? in B
        pula                    ; restore CC to A
        bcs     getchar_end
        tap
        clc                     ; clear carry
        pulb
        rts
getchar_end:
        tap
        sec                     ; set carry
        tba
        pulb

;;; Put character
;;; @param A
putchar:
	pshb
        psha
        tab                     ; char in B
        tpa
        psha                    ; save CC
	sei                     ; disable IRQ
        tba                     ; char in A
        pshx
        ldx     #tx_queue
        jsr     queue_add
        pulx
        tst     tx_int_control
        bne     putchar_end
        ldaa    #RX_INT_TX_INT  ; enable Tx interrupt
        staa    ACIA_control
        com     tx_int_control
putchar_end:
        pula                    ; restore CC
        tap
        pula
        pulb
        rts

        include "queue.inc"

isr_irq:
        ldab    ACIA_status
        bitb    #IRQF_bm
        beq     isr_irq_return
isr_irq_receive:
        bitb    #RDRF_bm
        beq     isr_irq_recv_end
        ldaa    ACIA_data       ; receive character
        ldx     #rx_queue
        jsr     queue_add
isr_irq_recv_end:
isr_irq_send:
        bitb    #TDRE_bm
        beq     isr_irq_send_end
        ldx     #tx_queue
        jsr     queue_remove
        bcc     isr_irq_send_empty
        staa    ACIA_data       ; send character
        bra     isr_irq_send_end
isr_irq_send_empty:
        ldaa    #RX_INT_TX_NO
        staa    ACIA_control    ; disable Tx interrupt
        clr     tx_int_control
isr_irq_send_end:
isr_irq_return:
        rti

        org     VEC_IRQ
        fdb     isr_irq

        org     VEC_RESET
        fdb     initialize
