        cpu     ins8070
        include "ins8070.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     0xDF00
        include "mc6850.inc"
ACIA_C: equ     0               ; ACIA control offset
ACIA_S: equ     0               ; ACIA status offset
ACIA_D: equ     1               ; ACIA data register offset

rx_queue_size:  equ     16
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm

        org     0x2000
rx_queue:
        ds      rx_queue_size

        org     0x1000
stack:
initialize:
        ld      sp, =stack
        ;; initialize receive queue
        ld      p2, =rx_queue
        ld      a, =rx_queue_size
        jsr     queue_init

        ;; initialize ACIA
        ld      p2, =ACIA
        ld      a, =CDS_RESET_gc ; Master reset
        st      a, ACIA_C, p2
        ld      a, =RX_INT_TX_NO
        st      a, ACIA_C, p2
        or      s, =S_IE        ; Enable IRQ

loop:
        ld      p2, =rx_queue
        and     s, =~S_IE       ; Disable IRQ
        jsr     queue_remove
        or      s, =S_IE        ; Enable IRQ
        bz      loop
echo:
        ld      a, e            ; A=data
        jsr     putchar
        xor     a, =0x0D        ; carriage return
        bnz     loop
        ld      a, =0x0A
        ld      e, a            ; E=newline
        bra     echo

putchar:
        pli     p2, =ACIA
        push    a
transmit_loop:
        ld      a, ACIA_S, p2
        and     a, =TDRE_bm
        bz      transmit_loop
transmit_data:
        pop     a
        st      a, ACIA_D, p2
        pop     p2
        ret

        include "queue.inc"

isr_irq:
        push    ea
        pli     p2, =ACIA
        ld      a, ACIA_S, p2
        ld      e, a
        and     a, =IRQF_bm
        bz      isr_irq_return
        ld      a, e
        and     a, =FERR_bm|OVRN_bm|PERR_bm
        bnz     isr_irq_recv_err
isr_irq_receive:
        ld      a, e
        and     a, =RDRF_bm
        bz      isr_irq_recv_end
        ld      a, ACIA_D, p2
        ld      p2, =rx_queue
        jsr     queue_add
isr_irq_recv_end:
isr_irq_return:
        pop     p2
        pop     ea
        or      s, =S_IE
        ret
isr_irq_recv_err:
        ld      a, ACIA_D, p2   ; clear errors
        bra     isr_irq_receive

        org     ORG_INTA
        jmp     isr_irq

        org     ORG_RESTART
        jmp     initialize
