        cpu     8085
        include "i8085.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:          equ     00H
USARTD:         equ     USART+0 ; Receive/Transmit data
USARTS:         equ     USART+1 ; Srtatus register
USARTC:         equ     USART+1 ; Control register
USARTRV:        equ     USART+2 ; Receive interrupt vector (ORG_*)
USARTTV:        equ     USART+3 ; Transmit interrupt vector (ORG_*)
        include "i8251.inc"
TXRX_ENABLE     equ     CMD_RTS_bm|CMD_DTR_bm|CMD_RxEN_bm|CMD_TxEN_bm
RXERR_RESET     equ     TXRX_ENABLE | CMD_ER_bm

        org     2000H
rx_queue_size:  equ     128
rx_queue:       ds      rx_queue_size
tx_queue_size:  equ     128
tx_queue:       ds      tx_queue_size
tx_intr_enable: db      1

        org     1000H
stack:  equ     $

        org     ORG_RESET
        jmp     init

        org     ORG_RST55
        jmp     isr_intr_rx

        org     ORG_RST65
        jmp     isr_intr_tx

init:
        lxi     sp, stack
        lxi     h, rx_queue
        mvi     b, rx_queue_size
        call     queue_init
        lxi     h, tx_queue
        mvi     b, tx_queue_size
        call     queue_init
init_usart:
        xra     a               ; clear A
        out     USARTC
        out     USARTC
        out     USARTC          ; safest way to sync mode
        mvi     a, CMD_IR_bm
        out     USARTC          ; reset
        nop
        nop
        mvi     a, MODE_STOP1_gc|MODE_LEN8_gc|MODE_BAUD_X16
        out     USARTC          ; async 1stop 8data x16
        nop
        nop
        mvi     a, TXRX_ENABLE
        out     USARTC          ; RTS/DTR, error reset, Rx enable, Tx enable
        mvi     a, ORG_RST55
        out     USARTRV         ; enable RxRDY interrupt using RST 5.5
        mvi     a, ORG_RESET
        out     USARTTV         ; disable TxRDY interrupt
        sta     tx_intr_enable

        rim
        ani     ~(SIM_M55|SIM_M65) ; enable RST 5.5/RST 5.6
        ori     SIM_MSE|SIM_R75
        sim
        ei

receive_loop:
        call    getchar
        jnc     receive_loop
echo_back:
        mov     b, a
        call    putchar         ; echo
        mvi     a, ' '          ; space
        call    putchar
        call    put_hex8        ; print in hex
        mvi     a, ' '          ; space
        call    putchar
        call    put_bin8        ; print in binary
        call    newline
        jmp     receive_loop

;;; Put newline
;;; @clobber A
newline:
        mvi     a, 0DH
        call    putchar
        mvi     a, 0AH
        jmp     putchar

;;; Print uint8_t in hex
;;; @param B uint8_t value to be printed in hex.
;;; @clobber A
put_hex8:
        mvi     a, '0'
        call    putchar
        mvi     a, 'x'
        call    putchar
        mov     a, b
        rrc
        rrc
        rrc
        rrc
        call    put_hex4
        mov     a, b
put_hex4:
        ani     0FH
        cpi     10
        jc      put_hex8_dec    ; A<10
        adi     'A'-10
        jmp     putchar
put_hex8_dec:
        adi     '0'
        jmp     putchar

;;; Print uint8_t in binary
;;; @param B uint8_t value to be printed in binary.
;;; @clobber A
put_bin8:
        push    b
        mvi     a, '0'
        call    putchar
        mvi     a, 'b'
        call    putchar
        mov     a, b
        call    put_bin4
        rlc
        call    put_bin4
        pop     b
        ret
put_bin4:
        call    put_bin2
        rlc
put_bin2:
        call    put_bin1
        rlc
put_bin1:
        mvi     c, '0'
        ora     a               ; chech MSB
        jp      put_bin0        ; MSB=0
        inr     c               ; MSB=1
put_bin0:
        mov     b, a
        mov     a, c
        call    putchar
        mov     a, b
        ret

;;; Get character
;;; @return A
;;; @return CC.C 0 if no character
getchar:
        push    h
        lxi     h, rx_queue
        di
        call    queue_remove
        ei
        pop     h
        ret

;;; Put character
;;; @param A
putchar:
        push    psw
        push    h
        lxi     h, tx_queue
putchar_retry:
        di
        call    queue_add
        ei
        jnc     putchar_retry   ; branch if queue is full
        pop     h
        lda     tx_intr_enable
        ora     a
        jnz     putchar_exit    ; already enabled
        mvi     a, ORG_RST65
        di
        out     USARTTV
        sta     tx_intr_enable
        ei
putchar_exit:
        pop     psw
        ret

        include "queue.inc"

isr_intr_rx:
        push    psw
        push    h
        push    b
        in      USARTS
        mov     b, a
        ani     ST_FE_bm|ST_OE_bm|ST_PE_bm
        jz      isr_intr_receive
        mvi     a, RXERR_RESET
        out     USARTC          ; reset error flags
isr_intr_receive:
        mov     a, b
        ani     ST_RxRDY_bm
        jz      isr_intr_rx_exit
        in      USARTD          ; receive character
        lxi     h, rx_queue
        call    queue_add
isr_intr_rx_exit:
        pop     b
        pop     h
        pop     psw
        ei
        ret

isr_intr_tx:
        push    psw
        push    h
        in      USARTS
        ani     ST_TxRDY_bm
        jz      isr_intr_tx_exit
        lxi     h, tx_queue
        call    queue_remove
        jnc     isr_intr_send_empty
        out     USARTD          ; send character
isr_intr_tx_exit:
        pop     h
        pop     psw
        ei
        ret
isr_intr_send_empty:
        xra     a
        out     USARTTV         ; disable Tx interrupt
        sta     tx_intr_enable
        jmp     isr_intr_tx_exit
