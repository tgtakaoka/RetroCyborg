        cpu     6809
        include "mc6809.inc"

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
stack:  equ     *

        org     $1000
initialize:
        lds     #stack
        ldx     #rx_queue
        ldb     #rx_queue_size
        lbsr    queue_init
        ldx     #tx_queue
        ldb     #tx_queue_size
        lbsr    queue_init
        ;; initialize ACIA
        lda     #CDS_RESET_gc   ; master reset
        sta     ACIA_control
        lda     #RX_INT_TX_NO
        sta     ACIA_control
        clr     tx_int_control  ; disabl Tx interrupt
        andcc   #~CC_IRQ        ; Clear IRQ mask

receive_loop:
        lbsr    getchar
        bcc     receive_loop
echo_back:
        tfr     a,b
        lbsr    putchar         ; echo
        lda     #$20            ; space
        lbsr    putchar
        bsr     put_hex8        ; print in hex
        lda     #$20
        lbsr    putchar
        bsr     put_bin8        ; print in binary
        lbsr    newline
        bra     receive_loop

;;; Put newline
;;; @clobber A
newline:
        lda     #$0d
        bsr     putchar
        lda     #$0a
        bra     putchar

;;; Print uint8_t in hex
;;; @param B uint8_t value to be printed in hex.
;;; @clobber A
put_hex8:
        lda     #'0'
        lbsr    putchar
        lda     #'x'
        lbsr    putchar
        tfr     b,a
        lsra
        lsra
        lsra
        lsra
        bsr     put_hex4
        tfr     b,a
put_hex4:
        anda    #$0f
        cmpa    #10
        blo     put_hex8_dec
        adda    #'A'-10
        lbra    putchar
put_hex8_dec:
        adda    #'0'
        lbra   putchar

;;; Print uint8_t in binary
;;; @param B uint8_t value to be printed in binary.
;;; @clobber A
put_bin8:
        pshs    b
        lda     #'0'
        lbsr    putchar
        lda     #'b'
        lbsr    putchar
        bsr     put_bin4
        lslb
        bsr     put_bin4
        puls    b,pc
put_bin4:
        bsr     put_bin2
        lslb
put_bin2:
        bsr     put_bin1
        lslb
put_bin1:
        lda     #'0'
        tstb                    ; chech MSB
        bpl     put_bin0        ; MSB=0
        inca                    ; MSB=1
put_bin0:
        lbra    putchar

;;; Get character
;;; @return A
;;; @return CC.C 0 if no character
getchar:
        orcc    #CC_CARRY       ; set carry
        pshs    x,cc
        ldx     #rx_queue
        orcc    #CC_IRQ         ; disable IRQ
        lbsr    queue_remove
        bcs     getchar_exit
        dec     ,s              ; clear carry
getchar_exit:
        puls    cc,x,pc

;;; Put character
;;; @param A
putchar:
        pshs    x,a,cc
        ldx     #tx_queue
putchar_retry:
        orcc    #CC_IRQ         ; disable IRQ
        lbsr    queue_add
        andcc   #~CC_IRQ        ; enable IRQ
        bcc     putchar_retry   ; branch if queue is full
        tst     tx_int_control
        bne     putchar_exit
        lda     #RX_INT_TX_INT  ; enable Tx interrupt
        sta     ACIA_control
        com     tx_int_control
putchar_exit:
        puls    cc,a,x,pc

        include "queue.inc"

isr_irq:
        ldb     ACIA_status
        bitb    #IRQF_bm
        beq     isr_irq_exit
        bitb    #FERR_bm|OVRN_bm|PERR_bm
        beq     isr_irq_receive
        lda     ACIA_data       ; reset error flags
isr_irq_receive:
        bitb    #RDRF_bm
        beq     isr_irq_send
        lda     ACIA_data       ; receive character
        ldx     #rx_queue
        lbsr    queue_add
isr_irq_send:
        bitb    #TDRE_bm
        beq     isr_irq_exit
        ldx     #tx_queue
        lbsr    queue_remove
        bcc     isr_irq_send_empty
        sta     ACIA_data       ; send character
isr_irq_exit:
        rti
isr_irq_send_empty:
        lda     #RX_INT_TX_NO
        sta     ACIA_control    ; disable Tx interrupt
        clr     tx_int_control
        rti

        org     VEC_IRQ
        fdb     isr_irq

        org     VEC_RESET
        fdb     initialize
