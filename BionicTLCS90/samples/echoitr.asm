        cpu     tlcs90
        include "tmp90c802.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:          equ     0FFF0H
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
        jp      init

        org     ORG_INT0
        jp      isr_intr
;        bit     ST_RxRDY_bp, (USARTS)
;        jp      nz, isr_intr_rx
;        reti

        org     ORG_INT1
        bit     ST_TxRDY_bp, (USARTS)
        jp      nz, isr_intr_tx
        reti

        org     0100H
init:
        ld      sp, stack
        ld      ix, rx_queue
        ld      b, rx_queue_size
        call     queue_init
        ld      ix, tx_queue
        ld      b, tx_queue_size
        call     queue_init
init_usart:
        ld      (USARTC), 0
        ld      (USARTC), 0
        ld      (USARTC), 0     ; safest way to sync mode
;;; reset
        ld      (USARTC), CMD_IR_bm
        nop
        nop
;;; async 1stop 8data x16
        ld      (USARTC), MODE_STOP1_gc|MODE_LEN8_gc|MODE_BAUD_X16
        nop
        nop
;;; RTS/DTR, error reset, Rx enable, Tx enable
;;; enable RxRDY interrupt using RST 5.5
        ld      (USARTC), TXRX_ENABLE
        ld      (USARTRV), ORG_INT0
        ld      (USARTTV), ORG_RESET ; disable TxRDY interrupt
;;; Enable INT0, INT1
        set     INTEH_IE0_bp, (INTEH)
;        set     INTEL_IE1_bp, (INTEL)
        ei

receive_loop:
        call    getchar
        jr      nc, receive_loop
        or      a, a
        jr      nz, echo_back
        swi                     ; A=0
echo_back:
        ld      b, a
        call    putchar         ; echo
        ld      a, ' '          ; space
        call    putchar
        call    put_hex8        ; print in hex
        ld      a, ' '          ; space
        call    putchar
        call    put_bin8        ; print in binary
        call    newline
        jr      receive_loop

;;; Put newline
;;; @clobber A
newline:
        ld      a, 0DH
        call    putchar
        ld      a, 0AH
        jp      putchar

;;; Print uint8_t in hex
;;; @param B uint8_t value to be printed in hex.
;;; @clobber A
put_hex8:
        ld      a, '0'
        call    putchar
        ld      a, 'x'
        call    putchar
        ld      a, b
        srla
        srla
        srla
        srla
        call    put_hex4
        ld      a, b
put_hex4:
        and     a, 0FH
        cp      a, 10
        jr      c, put_hex8_dec    ; A<10
        add     a, 'A'-10
        jp      putchar
put_hex8_dec:
        add     a, '0'
        jp      putchar

;;; Print uint8_t in binary
;;; @param B uint8_t value to be printed in binary.
;;; @clobber A
put_bin8:
        push    bc
        ld      a, '0'
        call    putchar
        ld      a, 'b'
        call    putchar
        ld      a, b
        call    put_bin4
        slla
        call    put_bin4
        pop     bc
        ret
put_bin4:
        call    put_bin2
        slla
put_bin2:
        call    put_bin1
        slla
put_bin1:
        ld      c, '0'
        or      a, a            ; chech MSB
        jr      pl, put_bin0    ; MSB=0
        inc     c               ; MSB=1
put_bin0:
        ld      b, a
        ld      a, c
        call    putchar
        ld      a, b
        ret

;;; Get character
;;; @return A
;;; @return CC.C 0 if no character
getchar:
        push    ix
        ld      ix, rx_queue
        di
        call    queue_remove
        ei
        pop     ix
        ret

;;; Put character
;;; @param A
putchar:
        push    af
        push    ix
        ld      ix, tx_queue
putchar_retry:
        di
        call    queue_add
        ei
        jr      nc, putchar_retry ; branch if queue is full
        pop     ix
        di
        ld      a, (USARTTV)
        or      a, a
        jr      nz, putchar_exit ; already enabled
        ld      (USARTTV), ORG_INT0
;        ld      (USARTTV), ORG_INT1
putchar_exit:
        ei
        pop     af
        ret

        include "queue.inc"

isr_intr:
        ld      a, (USARTS)
        bit     ST_RxRDY_bp, a
        jr      nz, isr_intr_rx
        bit     ST_TxRDY_bp, a
        jr      nz, isr_intr_tx
        reti

isr_intr_rx:
        ld      a, (USARTD)     ; receive character
        push    ix
        ld      ix, rx_queue
        call    queue_add
        pop     ix
        reti

isr_intr_tx:
        push    ix
        ld      ix, tx_queue
        call    queue_remove
        pop     ix
        jr      nc, isr_intr_send_empty
        ld      (USARTD), a     ; send character
        reti
isr_intr_send_empty:
        ld      (USARTTV), ORG_RESET ; disable Tx interrupt
        reti
