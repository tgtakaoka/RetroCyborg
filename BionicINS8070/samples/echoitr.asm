        cpu     ins8070
        include "ins8070.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     0xDF00
        include "mc6850.inc"
ACIA_C: equ     0               ; ACIA control offset
ACIA_S: equ     0               ; ACIA status offset
ACIA_D: equ     1               ; ACIA data register offset

rx_queue_size:  equ     16
tx_queue_size:  equ     48
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm
RX_INT_TX_INT:  equ     WSB_8N1_gc|RIEB_bm|TCB_EI_gc

        org     0x2000
rx_queue:
        ds      rx_queue_size
tx_queue:
        ds      tx_queue_size

        org     0xFF00
tx_int_control:
        ds      1

        org     0x1000
stack:
initialize:
        ld      sp, =stack
        ;; initialize queues
        ld      p2, =rx_queue
        ld      a, =rx_queue_size
        jsr     queue_init
        ld      p2, =tx_queue
        ld      a, =tx_queue_size
        jsr     queue_init

        ;; initialize ACIA
        ld      p2, =ACIA
        ld      a, =CDS_RESET_gc ; master reset
        st      a, ACIA_C, p2
        ld      a, =RX_INT_TX_NO
        st      a, ACIA_C, p2
        ld      a, =0
        st      a, tx_int_control ; disable Tx interrupt
        or      s, =S_IE          ; enable IRQ

loop:
        jsr     getchar
        bz      loop
        ld      a, e
        jsr     putchar         ; echo
        ld      a, =' '         ; space
        jsr     putchar
        ld      a, e
        jsr     put_hex8        ; print in hex
        ld      a, =' '         ; space
        jsr     putchar
        ld      a, e
        jsr     put_bin8        ; print in binary
        jsr     newline
        bra     loop

;;; Put newline
;;; @clobber A
newline:
        ld      a, =0x0D
        jsr     putchar
        ld      a, =0x0A
        jmp     putchar

;;; Print uint8_t in hex
;;; @param A uint8_t value to be printed in hex.
put_hex8:
        push    ea
        ld      a, ='0'
        jsr     putchar
        ld      a, ='x'
        jsr     putchar
        ld      a, 0, sp
        sr      a
        sr      a
        sr      a
        sr      a
        jsr     put_hex4
        ld      a, 0, sp
        jsr     put_hex4
        pop     ea
        ret
put_hex4:
        and     a, =0x0f
        sub     a, =10
        bp      put_hex4_hex
        add     a, ='0'+10
        jmp     putchar
put_hex4_hex:
        add     a, ='A'
        jmp     putchar

;;; Print uint8_t in binary
;;; @param A(save_a) uint8_t value to be printed in binary.
put_bin8:
        push    ea
        ld      a, ='0'
        jsr     putchar
        ld      a, ='b'
        jsr     putchar
        ld      a, 0, sp
        ld      e, a
        jsr     put_bin4
        jsr     put_bin4
        pop     ea
        ret
put_bin4:
        jsr     put_bin2
put_bin2:
        jsr     put_bin1
put_bin1:
        ld      a, e
        add     a, e
        ld      e, a            ; E<<=1
        ld      a, s
        bp      put_bin0        ; A:7=CY
        ld      a, ='1'
        jmp     putchar
put_bin0:
        ld      a, ='0'
        jmp     putchar

;;; Get character
;;; @clobber P2
;;; @return E char
;;; @return A 0 if no char received
getchar:
        ld      p2, =rx_queue
        and     s, =~S_IE       ; disable IRQ
        jsr     queue_remove
        or      s, =S_IE        ; enable IRQ
        ret

;;; Put character
;;; @param A char
;;; @clobber P2
putchar:
        push    ea
        ld      e, a
putchar_retry:
        ld      a, e
        ld      p2, =tx_queue
        and     s, =~S_IE       ; disable IRQ
        jsr     queue_add
        or      s, =S_IE        ; enable IRQ
        bz      putchar_retry   ; queue is full
        and     s, =~S_IE       ; disable IRQ
        ld      a, tx_int_control
        bnz     putchar_return
        ld      p2, =ACIA
        ld      a, =RX_INT_TX_INT ; enable Tx interrupt
        st      a, ACIA_C, p2
        st      a, tx_int_control
putchar_return:
        pop     ea
        or      s, =S_IE        ; enable IRQ
        ret

        include "queue.inc"

isr_irq:
        push    ea
        pli     p2, =ACIA
        ld      a, ACIA_S, p2
        ld      e, a            ; save ACIA status in E
        and     a, =IRQF_bm
        bz      isr_irq_return
        ld      a, e
        and     a, =FERR_bm|OVRN_bm|PERR_bm
        bz      isr_irq_receive
        ld      a, ACIA_D, p2   ; clear errors
isr_irq_receive:
        ld      a, e
        and     a, =RDRF_bm
        bz      isr_irq_recv_end
        ld      a, ACIA_D, p2   ; receive character
        ld      p2, =rx_queue
        jsr     queue_add
isr_irq_recv_end:
isr_irq_send:
        ld      a, e
        and     a, =TDRE_bm
        bz      isr_irq_send_end
        ld      p2, =tx_queue
        jsr     queue_remove
        ld      p2, =ACIA
        bz      isr_irq_send_empty
        ld      a, e
        st      a, ACIA_D, p2   ; send character
        bra     isr_irq_send_end
isr_irq_send_empty:
        ld      a, =RX_INT_TX_NO
        st      a, ACIA_C, p2   ; disable Tx interrupt
        ld      a, =0
        st      a, tx_int_control
isr_irq_send_end:
isr_irq_return:
        pop     p2
        pop     ea
        or      s, =S_IE
        ret

        org     ORG_INTA
        jmp     isr_irq

        org     ORG_RESTART
        jmp     initialize
