        cpu     ins8060
        include "ins8060.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     0xDF00
        include "mc6850.inc"
ACIA_C: equ     0               ; ACIA control offset
ACIA_S: equ     0               ; ACIA status offset
ACIA_D: equ     1               ; ACIA data register offset

RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm
RX_INT_TX_INT:  equ     WSB_8N1_gc|RIEB_bm|TCB_EI_gc

rx_queue_size:  equ     16
tx_queue_size:  equ     48

        org     0x2000
rx_queue:
        ds      rx_queue_size
tx_queue:
        ds      tx_queue_size

        org     0x1000
stack:  equ     $-1
initialize:
        ldi     (queue_init-1) & 0xFF
        xpal    P1
        ldi     (queue_init-1) >> 8
        xpah    P1
        ldi     rx_queue_size
        xppc    P1              ; call queue_init
        dw      rx_queue

        ;; initialize ACIA
        ldi     ACIA & 0xFF
        xpal    P1
        ldi     ACIA >> 8
        xpah    P1
        ldi     CDS_RESET_gc    ; Master reset
        st      ACIA_C(P1)
        ldi     RX_INT_TX_NO   ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        st      ACIA_C(P1)
        ien                     ; enable interrupt

loop:
        ldi     (queue_remove-1) & 0xFF
        xpal    P1
        ldi     (queue_remove-1) >> 8
        xpah    P1
        dint                    ; Disable IRQ
        xppc    P1              ; call queue_remote
        dw      rx_queue
        ien                     ; Enable IRQ
        jz      loop
echo:
        ldi     (putchar - 1) & 0xFF
        xpal    p1
        ldi     (putchar - 1) >> 8
        xpah    p1
        xppc    p1
        lde
        xri     0x0D            ; carriage return
        jnz     loop
        ldi     0x0A
        xae                     ; E=newline
        jmp     echo

;;; @param E char
putchar_exit:
        xppc    P1              ; return
putchar:
        ldi     ACIA >> 8
        xpah    P1
        st      @-1(P2)
        ldi     ACIA & 0xFF     ; save P1 and load P1
        xpal    P1
        st      @-1(P2)
transmit_loop:
        ld      ACIA_S(P1)
        ani     TDRE_bm
        jz      transmit_loop
transmit_data:
        lde
        st      ACIA_D(P1)
        ld      @1(P2)
        xpal    P1
        ld      @1(P2)          ; restore P1
        xpah    P1
        jmp     putchar_exit

        include "queue.inc"

isr_sensea_exit:
        ld      @1(P2)          ; pop P1
        xpal    P1
        ld      @1(P2)
        xpah    P1
        ld      @1(P2)          ; pop E
        xae
        ld      @1(P2)          ; pop D
        ien                     ; enable interrupt
        xppc    P3              ; return from interrupt
isr_sensea:
        st      @-1(P2)         ; save A
        lde
        st      @-1(P2)         ; save E
        ldi     ACIA >> 8
        xpah    P1
        st      @-1(P2)
        ldi     ACIA & 0xFF     ; save P1 and load P1
        xpal    P1
        st      @-1(P2)
        ld      ACIA_S(P1)
        xae
        lde
        ani     IRQF_bm
        jz      isr_sensea_exit
        lde
        ani     FERR_bm|OVRN_bm|PERR_bm
        jz      isr_receive
        ld      ACIA_D(P1)      ; clear errors
isr_receive:
        lde
        ani     RDRF_bm
        jz      isr_sensea_exit
        ld      ACIA_D(P1)
        xae                     ; E=char
        ldi     (queue_add - 1) >> 8
        xpah    P1
        ldi     (queue_add - 1) & 0xFF
        xpal    P1
        xppc    P1              ; call queue_add
        dw      rx_queue
        jmp     isr_sensea_exit

        org     ORG_RESTART
_stack: equ     (stack & 0xF000) | ((stack + 1) & 0x0FFF)
        ldi     _stack & 0xFF
        xpal    P2
        ldi     _stack >> 8
        xpah    P2
_isr_sensea:    equ     (isr_sensea & 0xF000) | ((isr_sensea-1) & 0x0FFF)
        ldi     _isr_sensea & 0xFF
        xpal    P3              ; setup interrupt entry P3
        ldi     _isr_sensea >> 8
        xpah    P3
_initialize:    equ     (initialize & 0xF000) | ((initialize-1) & 0x0FFF)
        ldi     _initialize & 0xFF
        xpal    P1
        ldi     _initialize >> 8
        xpah    P1
        xppc    P1
