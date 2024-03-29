        cpu     8085
        include "i8085.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:  equ     00H
USARTD: equ     USART+0         ; Data register
USARTS: equ     USART+1         ; Status register
USARTC: equ     USART+1         ; Control register
        include "i8251.inc"

        org     1000H
stack:  equ     $

        org     ORG_RESET
        jmp     init_usart

        org     0100H
init_usart:
        lxi     sp, stack
        xra     A               ; clear A
        out     USARTC
        out     USARTC
        out     USARTC          ; safest way to sync mode
        mvi     A, CMD_IR_bm
        out     USARTC          ; reset
        nop
        nop
        mvi     A, MODE_STOP1_gc|MODE_LEN8_gc|MODE_BAUD_X16
        out     USARTC          ; async 1stop 8data x16
        nop
        nop
        mvi     A, CMD_RTS_bm|CMD_DTR_bm|CMD_ER_bm|CMD_RxEN_bm|CMD_TxEN_bm
        out     USARTC          ; RTS/DTR, error reset, Rx enable, Tx enable

        call    arith
        hlt

putchar:
        push    PSW
putchar_loop:
        in      USARTS
        ani     ST_TxRDY_bm
        jz      putchar_loop
        pop     PSW
        out     USARTD
        ret

newline:
        push    PSW
        mvi     A, 0DH
        call    putchar
        mvi     A, 0AH
        call    putchar
        pop     PSW
        ret

putspace:
        push    PSW
        mvi     A, ' '
        call    putchar
        pop     PSW
        ret

;;; Print unsigned 16-bit integer as decimal
;;; @param HL: value
;;; @clobber HL
print_uint16:
        push    PSW
print_uint16_inner:
        push    B
        push    D
        mov     B, H
        mov     C, L
        mov     A, B
        ora     C
        jz      print_uint16_zero
        call    print_uint16_loop
        pop     D
        pop     B
        pop     PSW
        ret
print_uint16_loop:
        mov     A, B
        ora     C
        rz
        lxi     D, 10
        call    udiv16
        push    H               ; push reminder
        call    print_uint16_loop
        pop     H               ; pop reminder
        mov     A, L
        adi     '0'
        jmp     putchar
print_uint16_zero:
        mvi     A, '0'
        call    putchar
        pop     D
        pop     B
        pop     PSW
        ret

;;; Print signed 16-bit integer as decimal
;;; @param HL: value
;;; @clobber HL
print_int16:
        push    PSW
        mov     A, H
        ora     A
        jp      print_uint16_inner
        mvi     A, '-'
        call    putchar
        mov     A, L
        cma
        mov     L, A
        mov     A, H
        cma
        mov     H, A
        inx     H               ; HL=-value
        jmp     print_uint16_inner

expr:
        push    PSW
        ldax    B
        mov     L, A
        inx     B
        ldax    B
        mov     H, A
        dcx     B               ; HL=@BC
        call    print_int16
        call    putspace
        pop     PSW
        call    putchar
        call    putspace
        ldax    D
        mov     L, A
        inx     D
        ldax    D
        mov     H, A
        dcx     D
        jmp     print_int16     ; HL=@DE

answer:
        call    putspace
        mvi     A, '='
        call    putchar
        call    putspace
        lhld    vA
        call    print_int16
        jmp     newline

comp:
        lxi     B, vA
        lxi     D, vB
        call    cmpsi2
        jz      comp_eq
        jp      comp_gt
        jm      comp_lt
        mvi     A, '?'
        jmp     comp_out
comp_gt:
        mvi     A, '>'
        jmp     comp_out
comp_eq:
        mvi     A, '='
        jmp     comp_out
comp_lt:
        mvi     A, '<'
comp_out:
        call    expr
        jmp     newline

        org     0200H

vA:     ds      2
vB:     ds      2

        org     1000H

arith:
        lxi     B, vA
        lxi     D, vB

        lxi     H, 0
        shld    vA
        lxi     H, -28000
        shld    vB
        mvi     A, '-'
        call    expr
        call    negsi2
        call    answer          ; 28000

        lxi     H, 0
        shld    vA
        lxi     H, 28000
        shld    vB
        mvi     A, '-'
        call    expr
        call    negsi2
        call    answer          ; -28000

        lxi     H, 18000
        shld    vA
        lxi     H, 28000
        shld    vB
        mvi     A, '+'
        call    expr
        call    addsi2
        call    answer          ; -19536

        lxi     H, 18000
        shld    vA
        lxi     H, -18000
        shld    vB
        mvi     A, '+'
        call    expr
        call    addsi2
        call    answer          ; 0

        lxi     H, -18000
        shld    vA
        lxi     H, -18000
        shld    vB
        mvi     A, '+'
        call    expr
        call    addsi2
        call    answer          ; 29536

        lxi     H, -18000
        shld    vA
        lxi     H, -28000
        shld    vB
        mvi     A, '-'
        call    expr
        call    subsi2
        call    answer          ; -10000

        lxi     H, 100
        shld    vA
        lxi     H, 300
        shld    vB
        mvi     A, '*'
        call    expr
        call    mulsi2
        call    answer          ; 30000

        lxi     H, 300
        shld    vA
        lxi     H, -200
        shld    vB
        mvi     A, '*'
        call    expr
        call    mulsi2
        call    answer          ; 5536

        lxi     H, 100
        shld    vA
        lxi     H, -300
        shld    vB
        mvi     A, '*'
        call    expr
        call    mulsi2
        call    answer          ; -30000

        lxi     H, -200
        shld    vA
        lxi     H, -100
        shld    vB
        mvi     A, '*'
        call    expr
        call    mulsi2
        call    answer          ; 20000

        lxi     H, 30000
        shld    vA
        lxi     H, 100
        shld    vB
        mvi     A, '/'
        call    expr
        call    udivsi2
        call    answer          ; 30

        lxi     H, -200
        shld    vA
        lxi     H, 100
        shld    vB
        mvi     A, '/'
        call    expr
        call    divsi2
        call    answer          ; -2

        lxi     H, -30000
        shld    vA
        lxi     H, -200
        shld    vB
        mvi     A, '/'
        call    expr
        call    divsi2
        call    answer          ; 150

        lxi     H, -30000
        shld    vA
        lxi     H, 78
        shld    vB
        mvi     A, '/'
        call    expr
        call    divsi2
        call    answer          ; -384

        lxi     H, -48
        shld    vA
        lxi     H, 30
        shld    vB
        call    comp

        lxi     H, 30
        shld    vA
        lxi     H, -48
        shld    vB
        call    comp

        lxi     H, 5000
        shld    vA
        lxi     H, 4000
        shld    vB
        call    comp

        lxi     H, 5000
        shld    vB
        call    comp

        lxi     H, 4000
        shld    vA
        call    comp

        lxi     H, -5000
        shld    vA
        lxi     H, -4000
        shld    vB
        call    comp

        lxi     H, -5000
        shld    vB
        call    comp

        lxi     H, -4000
        shld    vA
        call    comp

        lxi     H, 32700
        shld    vA
        lxi     H, 32600
        shld    vB
        call    comp

        lxi     H, 32700
        shld    vB
        call    comp

        lxi     H, 32600
        shld    vA
        call    comp

        lxi     H, -32700
        shld    vA
        lxi     H, -32600
        shld    vB
        call    comp

        lxi     H, -32700
        shld    vB
        call    comp

        lxi     H, -32600
        shld    vA
        call    comp

        lxi     H, 18000
        shld    vA
        lxi     H, -28000
        shld    vB
        call    comp

        lxi     H, 18000
        shld    vB
        call    comp

        lxi     H, -28000
        shld    vA
        call    comp

        ret

        include "arith.inc"

        end
