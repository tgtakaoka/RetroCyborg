
        include "ins8060.inc"

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
tx_int_control:
        ds      1

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
        ldi     tx_queue_size
        xppc    P1              ; call queue init
        dw      tx_queue

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

        ldi     tx_int_control & 0xFF
        xpal    P1
        ldi     tx_int_control >> 8
        xpah    P1
        ldi     0
        st      0(P1)           ; disable Tx interrupt
        ien                     ; enable interrupt

loop:
        ldi     (getchar-1) & 0xff
        xpal    P1
        ldi     (getchar-1) >> 8
        xpah    P1
wait_char:
        xppc    P1              ; call getchar
        jz      wait_char
        ldi     (putchar-1) & 0xff
        xpal    P1
        ldi     (putchar-1) >> 8
        xpah    P1
        lde
        xppc    P1              ; call putchar
        ldi     ' '             ; space
        xppc    P1              ; call putchar
        ldi     (put_hex8-1) & 0xff
        xpal    P1
        ldi     (put_hex8-1) >> 8
        xpah    P1
        xppc    P1              ; call put_hex8
        ldi     (putchar-1) & 0xff
        xpal    P1
        ldi     (putchar-1) >> 8
        xpah    P1
        ldi     ' '             ; space
        xppc    P1              ; call putchar
        ldi     (put_bin8-1) & 0xff
        xpal    P1
        ldi     (put_bin8-1) >> 8
        xpah    P1
        xppc    P1              ; call put_bin8
        ldi     (putchar-1) & 0xff
        xpal    P1
        ldi     (putchar-1) >> 8
        xpah    P1
        ldi     '\r'
        xppc    P1              ; call putchar
        ldi     '\n'
        xppc    P1              ; call putchar
        jmp     loop

;;; Print uint8_t in hex
;;; @param E uint8_t value to be printed in hex.
put_hex8_exit:
        ld      @1(P2)          ; pop P1
        xpal    P1
        ld      @1(P2)
        xpah    P1
        xppc    P1
put_hex8:
        ldi     (putchar-1) >> 8
        xpah    P1
        st      @-1(P2)         ; push P1
        ldi     (putchar-1) & 0xff
        xpal    P1
        st      @-1(P2)
        ldi     '0'
        xppc    P1              ; call putchar
        ldi     'x'
        xppc    P1              ; call putchar
        ldi     (put_hex4-1) & 0xff
        xpal    P1
        ldi     (put_hex4-1) >> 8
        xpah    P1
        lde
        st      @-1(P2)         ; push E
        sr
        sr
        sr
        sr
        xae
        xppc    P1              ; call put_hex4
        ld      @1(P2)          ; pop E
        xae
        xppc    P1              ; call put_hex4
        jmp     put_hex8_exit

put_hex4_exit:
        ld      @1(P2)          ; pop P1
        xpal    P1
        ld      @1(P2)
        xpah    P1
        ld      @1(P2)          ; pop E
        xae
        xppc    P1
put_hex4:
        lde
        st      @-1(P2)         ; push E
        ani     0xf
        xae                     ; E=4 bit data
        ldi     (putchar-1) >> 8
        xpah    P1
        st      @-1(P2)         ; push P1
        ldi     (putchar-1) & 0xff
        xpal    P1
        st      @-1(P2)
        lde
        scl
        cai     10
        csa
        ccl
        jp      put_hex4_dec    ; branch if A<10
        lde
        adi     'A'-10
        xppc    P1              ; call putchar
        jmp     put_hex4_exit
put_hex4_dec:
        lde
        adi     '0'
        xppc    P1              ; call putchar
        jmp     put_hex4_exit

;;; Print uint8_t in binary
;;; @param E uint8_t value to be printed in binary.
put_bin8_exit:
        ld      @1(P2)          ; pop P1
        xpal    P1
        ld      @1(P2)
        xpah    P1
        ld      @1(P2)          ; pop E
        xae
        xppc    P1
put_bin8:
        lde
        st      @-1(P2)         ; push E
        ldi     (putchar-1) >> 8
        xpah    P1
        st      @-1(P2)         ; push P1
        ldi     (putchar-1) & 0xff
        xpal    P1
        st      @-1(P2)
        ldi     '0'
        xppc    P1              ; call putchar
        ldi     'b'
        xppc    P1              ; call putchar
        ldi     8
        st      @-1(P2)         ; bit count
put_bin8_loop:
        lde
        jp      put_bin8_zero
        ldi     '1'
        jmp     put_bin8_char
put_bin8_zero:
        ldi     '0'
put_bin8_char:
        xppc    P1              ; call putchar
        lde
        ade
        xae                     ; E<<=1
        dld     0(P2)           ; decrement bit count
        jnz     put_bin8_loop
        ld      @1(P2)          ; discard bit count
        jmp     put_bin8_exit

;;; Get character
;;; @return E char
;;; @return A 0 if no char received
getchar_exit:
        ld      @1(P2)          ; pop P1
        xpal    P1
        ld      @1(P2)
        xpah    P1
        ld      @1(P2)          ; pop return value
        xppc    P1
getchar:
        st      @-1(P2)         ; for return value
        ldi     (queue_remove-1) >> 8
        xpah    P1
        st      @-1(P2)         ; push P1
        ldi     (queue_remove-1) & 0xff
        xpal    P1
        st      @-1(P2)
        dint                    ; disable IRQ
        xppc    P1              ; call queue_remove
        dw      rx_queue
        ien                     ; enable IRQ
        st      2(P2)           ; save return value
        jmp     getchar_exit

;;; Put character
;;; @param A char
putchar_exit:
        ld      @1(P2)          ; pop P1
        xpal    P1
        ld      @1(P2)
        xpah    P1
        ld      @1(P2)          ; pop E
        xae
        ld      @1(P2)          ; pop A
        xppc    P1
putchar:
        st      @-1(P2)         ; push A
        lde
        st      @-1(P2)         ; push E
        ldi     (queue_add-1) >> 8
        xpah    P1
        st      @-1(P2)         ; push P1
        ldi     (queue_add-1) & 0xff
        xpal    P1
        st      @-1(P2)
        ld      3(P2)           ; restore char
        xae
putchar_retry:
        dint                    ; disable interrupt
        xppc    P1              ; call queue_add
        dw      tx_queue
        ien                     ; enable interrupt
        jz      putchar_retry   ; queue is full
        dint                    ; disable interrupt
        ldi     tx_int_control & 0xff
        xpal    p1
        ldi     tx_int_control >> 8
        xpah    p1
        ld      0(P1)           ; check tx interrupt state
        jnz     putchar_return
        ild     0(P1)           ; Tx interrupt is enabled
        ldi     ACIA & 0xff
        xpal    P1
        ldi     ACIA >> 8
        xpah    P1
        ldi     RX_INT_TX_INT   ; enable Tx interrupt
        st      ACIA_C(P1)
putchar_return:
        ien                     ; enable interrupt
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
        ani     FERR_bm|OVRN_bm|PERR_bm
        jz      isr_receive
        ld      ACIA_D(P1)      ; clear errors
isr_receive:
        lde
        ani     RDRF_bm
        jz      isr_send
        ld      ACIA_D(P1)      ; receive character
        xae                     ; E=char
        st      @-1(P2)         ; push ACIA status
        ldi     (queue_add - 1) & 0xFF
        xpal    P1
        ldi     (queue_add - 1) >> 8
        xpah    P1
        xppc    P1              ; call queue_add
        dw      rx_queue
        ld      @1(P2)          ; pop ACIA status
        xae
isr_send:
        lde
        ani     TDRE_bm
        jz      isr_sensea_exit
        ldi     (queue_remove - 1) & 0xFF
        xpal    P1
        ldi     (queue_remove - 1) >> 8
        xpah    P1
        xppc    P1              ; call queue_remove
        dw      tx_queue
        jz      isr_send_empty
        ldi     ACIA & 0xff
        xpal    P1
        ldi     ACIA >> 8
        xpah    P1
        lde
        st      ACIA_D(P1)      ; send character
        jmp     isr_sensea_exit
isr_send_empty:
        ldi     ACIA & 0xff
        xpal    P1
        ldi     ACIA >> 8
        xpah    P1
        ldi     RX_INT_TX_NO
        st      ACIA_C(P1)      ; disable Tx interrupt
        ldi     tx_int_control & 0xff
        xpal    P1
        ldi     tx_int_control >> 8
        xpah    P1
        ldi     0
        st      0(P1)           ; mark Tx interrupt disabled
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
