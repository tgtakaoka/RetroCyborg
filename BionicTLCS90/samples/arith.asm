        cpu     tlcs90
        include "tmp90c802.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:  equ     0FFF0H
USARTD: equ     USART+0         ; Data register
USARTS: equ     USART+1         ; Status register
USARTC: equ     USART+1         ; Control register
        include "i8251.inc"

        org     1000H
stack:  equ     $

        org     ORG_RESET
        ld      SP, stack
        jp      init_usart

        org     0100H
init_usart:
        ld      (USARTC), 0
        ld      (USARTC), 0
        ld      (USARTC), 0     ; safest way to sync mode
;;; reset
        ld      (USARTC), CMD_IR_bm
        nop
        nop
;;;  async 1stop 8data x16
        ld      (USARTC), MODE_STOP1_gc|MODE_LEN8_gc|MODE_BAUD_X16
        nop
        nop
;;;  RTS/DTR, error reset, Rx enable, Tx enable
        ld      (USARTC), CMD_RTS_bm|CMD_DTR_bm|CMD_ER_bm|CMD_RxEN_bm|CMD_TxEN_bm

        call    arith
        ld      A, 0
        swi

putchar:
        bit     ST_TxRDY_bp, (USARTS)
        jr      Z, putchar
        ld      (USARTD), A
        ret

newline:
        push    AF
        ld      A, 0DH
        call    putchar
        ld      A, 0AH
        call    putchar
        pop     AF
        ret

putspace:
        push    AF
        ld      A, ' '
        call    putchar
        pop     AF
        ret

;;; Print unsigned 16-bit integer as decimal
;;; @param HL: value
;;; @clobber HL
print_uint16:
        push    AF
print_uint16_inner:
        ld      A, H
        or      A, L
        jr      Z, print_uint16_zero
        push    BC
        push    DE
        ld      BC, HL
        call    print_uint16_loop
        pop     DE
        pop     BC
        pop     AF
        ret
print_uint16_loop:
        ld      A, B
        or      A, C
        ret     Z
        ld      DE, 10
        call    udiv16
        push    HL              ; push reminder
        call    print_uint16_loop
        pop     HL              ; pop reminder
        ld      A, L
        add     A, '0'
        jp      putchar
print_uint16_zero:
        ld      A, '0'
        call    putchar
        pop     AF
        ret

;;; Print signed 16-bit integer as decimal
;;; @param HL: value
;;; @clobber HL
print_int16:
        push    AF
        bit     7, H
        jr      Z, print_uint16_inner
        ld      A, '-'
        call    putchar
        xor     HL, 0FFFFH
        inc     HL              ; HL=-value
        jr      print_uint16_inner

expr:
        push    AF
        ld      HL, (IX)
        call    print_int16
        call    putspace
        pop     AF
        call    putchar
        call    putspace
        ld      HL, (IY)
        jr      print_int16

answer:
        call    putspace
        ld      A, '='
        call    putchar
        call    putspace
        ld      HL, (vA)
        call    print_int16
        jp      newline

comp:
        ld      IX, vA
        ld      IY, vB
        call    cmpsi2
        jr      Z, comp_eq
        jr      GT, comp_gt
        jr      LT, comp_lt
        ld      A, '?'
        jr      comp_out
comp_gt:
        ld      A, '>'
        jr      comp_out
comp_eq:
        ld      A, '='
        jr      comp_out
comp_lt:
        ld      A, '<'
comp_out:
        call    expr
        jp      newline

        org     0200H

vA:     ds      2
vB:     ds      2

        org     1000H

arith:
        ld      IX, vA
        ld      IY, vB

        ldw     (vA), 0
        ldw     (vB), -28000
        ld      A, '-'
        call    expr
        call    negsi2
        call    answer          ; 28000

        ldw     (vA), 0
        ldw     (vB), 28000
        ld      A, '-'
        call    expr
        call    negsi2
        call    answer          ; -28000

        ldw     (vA), 18000
        ldw     (vB), 28000
        ld      A, '+'
        call    expr
        call    addsi2
        call    answer          ; -19536

        ldw     (vA), 18000
        ldw     (vB), -18000
        ld      A, '+'
        call    expr
        call    addsi2
        call    answer          ; 0

        ldw     (vA), -18000
        ldw     (vB), -18000
        ld      A, '+'
        call    expr
        call    addsi2
        call    answer          ; 29536

        ldw     (vA), -18000
        ldw     (vB), -28000
        ld      A, '-'
        call    expr
        call    subsi2
        call    answer          ; 10000

        ldw     (vA), 100
        ldw     (vB), 300
        ld      A, '*'
        call    expr
        call    mulsi2
        call    answer          ; 30000

        ldw     (vA), 300
        ldw     (vB), -200
        ld      A, '*'
        call    expr
        call    mulsi2
        call    answer          ; 5536

        ldw     (vA), 100
        ldw     (vB), -300
        ld      A, '*'
        call    expr
        call    mulsi2
        call    answer          ; -30000

        ldw     (vA), -200
        ldw     (vB), -100
        ld      A, '*'
        call    expr
        call    mulsi2
        call    answer          ; 20000

        ldw     (vA), 30000
        ldw     (vB), 100
        ld      A, '/'
        call    expr
        call    udivsi2
        call    answer          ; 300

        ldw     (vA), -200
        ldw     (vB), 100
        ld      A, '/'
        call    expr
        call    divsi2
        call    answer          ; -2

        ldw     (vA), -30000
        ldw     (vB), -200
        ld      A, '/'
        call    expr
        call    divsi2
        call    answer          ; 150

        ldw     (vA), -30000
        ldw     (vB), 78
        ld      A, '/'
        call    expr
        call    divsi2
        call    answer          ; -384

        ldw     (vA), -48
        ldw     (vB), 30
        call    comp

        ldw     (vA), 30
        ldw     (vB), -48
        call    comp

        ldw     (vA), 5000
        ldw     (vB), 4000
        call    comp

        ldw     (vB), 5000
        call    comp

        ldw     (vA), 4000
        call    comp

        ldw     (vA), -5000
        ldw     (vB), -4000
        call    comp

        ldw     (vB), -5000
        call    comp

        ldw     (vA), -4000
        call    comp

        ldw     (vA), 32700
        ldw     (vB), 32600
        call    comp

        ldw     (vB), 32700
        call    comp

        ldw     (vA), 32600
        call    comp

        ldw     (vA), -32700
        ldw     (vB), -32600
        call    comp

        ldw     (vB), -32700
        call    comp

        ldw     (vA), -32600
        call    comp

        ldw     (vA), 18000
        ldw     (vB), -28000
        call    comp

        ldw     (vB), 18000
        call    comp

        ldw     (vA), -28000
        call    comp

        ret

        include "arith.inc"

        end
