;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     z8
        option  "reg-alias", "disable"

        include "z8.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:          equ     %FF00
USARTD:         equ     USART+0 ; Receive/Transmit data
USARTS:         equ     USART+1 ; Srtatus register
USARTC:         equ     USART+1 ; Control register
USARTRI:        equ     USART+2 ; Receive interrupt name (IRQ0~2)
USARTTI:        equ     USART+3 ; Transmit interrupt name (IRQ0~2)
        include "i8251.inc"

TXRX_ENABLE:    equ     CMD_RTS_bm LOR CMD_DTR_bm LOR CMD_RxEN_bm LOR CMD_TxEN_bm
RXERR_RESET:    equ     TXRX_ENABLE LOR CMD_ER_bm

        org     %2000
rx_queue_size:  equ     128
rx_queue:       ds      rx_queue_size
tx_queue_size:  equ     128
tx_queue:       ds      tx_queue_size

tx_intr_enable: equ     %20     ; R32

        org     %1000
stack:  equ     $

        org     VEC_IRQ0
        dw      isr_intr_rx

        org     VEC_IRQ1
        dw      isr_intr_tx

        org     ORG_RESET
init_config:
        srp     #P01M LAND %F0
        setrp   P01M LAND %F0
        ;; Stack is on external memory
        ld      P01M, #P01M_P0ADDR LOR P01M_P1DATA
        ld      P2M, #%FF       ; Port 2 is input
        srp     #%10
        setrp   %10
        ld      SPH, #HIGH stack
        ld      SPL, #LOW stack
        ld      r2, #HIGH rx_queue
        ld      r3, #LOW rx_queue
        ld      r1, #rx_queue_size
        call    queue_init
        ld      r2, #HIGH tx_queue
        ld      r3, #LOW tx_queue
        ld      r1, #tx_queue_size
        call    queue_init

init_usart:
        ld      r12, #HIGH USARTC
        ld      r13, #LOW USARTC
        clr     r0
        lde     @rr12, r0
        lde     @rr12, r0
        lde     @rr12, r0       ; safest way to sync mode
        ld      r0, #CMD_IR_bm
        lde     @rr12, r0       ; reset
        nop
        nop
        ld      r0, # MODE_STOP1_gc LOR MODE_LEN8_gc LOR MODE_BAUD_X16
        lde     @rr12, r0       ; async 1stop 8data x16
        nop
        nop
        ld      r0, #TXRX_ENABLE
        lde     @rr12, r0 ; RTS/DTR, error reset, Rx enable, Tx enable
        ld      r0, #INTR_IRQ0
        ld      r13, #LOW USARTRI
        lde     @rr12, r0 ; enable RxRDY interrupt using IRQ0
        ld      r0, #INTR_NONE
        ld      r13, #LOW USARTTI
        lde     @rr12, r0 ; disable TxRDY interrupt
        ld      r13, #LOW USARTS
        clr     tx_intr_enable
        ld      r10, #HIGH USARTD
        ld      r11, #LOW USARTD

        ld      IPR, #IPR_BCA LOR IPR_B02 LOR IPR_C14 LOR IPR_A35
        ;; enable IRQ0 and IRQ1
        ld      IMR, #IMR_IRQ0 LOR IMR_IRQ1
        ei

receive_loop:
        call    getchar
        jr      nc, receive_loop
echo_back:
        ld      r1, r0          ; save letter
        call    putchar         ; echo
        ld      r0, #' '        ; space
        call    putchar
        call    put_hex8        ; print in hex
        ld      r0, #' '        ; space
        call    putchar
        call    put_bin8        ; print in binary
        call    newline
        jr      receive_loop

;;; Put newline
;;; @clobber r0
newline:
        ld      r0, #%0D
        call    putchar
        ld      r0, #%0A
        jr      putchar

;;; Print uint8_t in hex
;;; @param r1 uint8_t value to be printed in hex.
;;; @clobber r0
put_hex8:
        ld      r0, #'0'
        call    putchar
        ld      r0, #'x'
        call    putchar
        ld      r0, r1
        swap    r0
        call    put_hex4
        ld      r0, r1
put_hex4:
        and     r0, #%F
        cp      r0, #10
        jr      c, put_hex8_dec ; A<10
        add     r0, #'A'-10
        jr      putchar
put_hex8_dec:
        add     r0, #'0'
        jr      putchar

;;; Print uint8_t in binary
;;; @param r1 uint8_t value to be printed in binary.
;;; @clobber r0
put_bin8:
        push    r4
        ld      r0, #'0'
        call    putchar
        ld      r0, #'b'
        call    putchar
        ld      r4, r1
        call    put_bin4
        rl      r4
        call    put_bin4
        pop     r4
        ret
put_bin4:
        call    put_bin2
        rl      r4
put_bin2:
        call    put_bin1
        rl      r4
put_bin1:
        ld      r0, #'0'
        or      r4, r4
        jp      pl, put_bin0    ; MSB=0
        ld      r0, #'1'        ; MSB=1
put_bin0:
        jr      putchar

;;; Get character
;;; @return r0
;;; @return FLAGS.C 0 if no character
getchar:
        push    r3
        push    r2
        ld      r2, #HIGH rx_queue
        ld      r3, #LOW rx_queue
        di
        call    queue_remove
        ei
        pop     r2
        pop     r3
        ret

;;; Put character
;;; @param r0
putchar:
        push    r0
        push    r3
        push    r2
        ld      r2, #HIGH tx_queue
        ld      r3, #LOW tx_queue
putchar_retry:
        di
        call    queue_add
        ei
        jr      nc, putchar_retry ; branch if queue is full
        pop     r2
        pop     r3
        tm      tx_intr_enable, #1
        jr      nz, putchar_exit ; already enabled
        ld      r0, #INTR_IRQ1
        push    r13
        di
        ld      r13, #LOW USARTTI
        lde     @rr12, r0
        pop     r13
        or      tx_intr_enable, #1
        ei
putchar_exit:
        pop     r0
        ret

        include "queue.inc"

        setrp   -1
isr_intr_rx:
        push    r0
        push    r1
        lde     r0, @rr12       ; USARTS
        ld      r1, r0
        and     r0, #ST_FE_bm LOR ST_OE_bm LOR ST_PE_bm
        jr      z, isr_intr_receive
        ld      r0, #RXERR_RESET
        lde     @rr12, r0       ; reset error flags
isr_intr_receive:
        and     r1, #ST_RxRDY_bm
        jr      z, isr_intr_rx_exit
        lde     r0, @rr10       ; USARTD
        push    r3
        push    r2
        ld      r2, #HIGH rx_queue
        ld      r3, #LOW rx_queue
        call    queue_add
        pop     r2
        pop     r3
isr_intr_rx_exit:
        pop     r1
        pop     r0
        iret

isr_intr_tx:
        push    r0
        lde     r0, @rr12       ; USARTS
        and     r0, #ST_TxRDY_bm
        jr      z, isr_intr_tx_exit
        push    r3
        push    r2
        ld      r2, #HIGH tx_queue
        ld      r3, #LOW tx_queue
        call    queue_remove
        pop     r2
        pop     r3
        jr      nc, isr_intr_send_empty
        lde     @rr10, r0       ; USARTD
isr_intr_tx_exit:
        pop     r0
        iret
isr_intr_send_empty:
        ld      r0, #INTR_NONE
        push    r13
        ld      r13, #LOW USARTTI
        lde     @rr12, r0       ; disable Tx interrupt
        pop     r13
        clr     tx_intr_enable
        pop     r0
        iret
