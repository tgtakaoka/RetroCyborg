        cpu     ins8060
        include "ins8060.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     0xDF00
        include "mc6850.inc"
ACIA_C: equ     0               ; ACIA control offset
ACIA_S: equ     0               ; ACIA status offset
ACIA_D: equ     1               ; ACIA data register offset

        org     0x1000
stack:  equ     $-1
initialize:
        ldi     ACIA & 0xFF
        xpal    P1
        ldi     ACIA >> 8
        xpah    P1
        ldi     CDS_RESET_gc    ; Master reset
        st      ACIA_C(P1)
        ldi     WSB_8N1_gc      ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        st      ACIA_C(P1)

receive_loop:
        ld      ACIA_S(P1)
        ani     RDRF_bm
        jz      receive_loop
receive_data:
        ld      ACIA_D(P1)
        xae                     ; E=data
transmit_loop:
        ld      ACIA_S(P1)
        ani     TDRE_bm
        jz      transmit_loop
transmit_data:
        xae                     ; A=data
        st      ACIA_D(P1)
        xri     0x0D
        jnz     receive_loop
        ldi     0x0A
        xae                     ; E=data
        jmp     transmit_loop

        org     ORG_RESTART
_stack: equ     (stack & 0xF000) | ((stack + 1) & 0x0FFF)
        ldi     (_stack) & 0xFF
        xpal    P2
        ldi     _stack >> 8
        xpah    P2
_initialize:    equ     (initialize & 0xF000) | ((initialize-1) & 0x0FFF)
        ldi     _initialize & 0xFF
        xpal    P3
        ldi     _initialize >> 8
        xpah    P3
        xppc    P3
