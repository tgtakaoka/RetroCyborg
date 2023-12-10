;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     f3850

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:  equ     0F0H
USARTD: equ     USART+0         ; Data register
USARTS: equ     USART+1         ; Status register
USARTC: equ     USART+1         ; Control register
        include "i8251.inc"

        org     0
        jmp     init_usart

        org     H'1000'
stack:
        org     0200H
        include "stack.inc"

init_usart:
        pi      init_stack
        lis     0
        out     USARTC
        out     USARTC
        out     USARTC          ; safest way to sync mode
;;; reset
        li      CMD_IR_bm
        out     USARTC
        nop
        nop
;;;  async 1stop 8data x16
        li      MODE_STOP1_gc|MODE_LEN8_gc|MODE_BAUD_X16
        out     USARTC
        nop
        nop
;;;  RTS/DTR, error reset, Rx enable, Tx enable
        li      CMD_RTS_bm|CMD_DTR_bm|CMD_ER_bm|CMD_RxEN_bm|CMD_TxEN_bm
        out     USARTC

        pi      call
        da      arith
        dc      H'2F'

;;; Print out char
;;; @param 0 char
;;; @clobber A
putchar:
        in      USARTS
        ni      ST_TxRDY_bm
        bz      putchar
        lr      A, 0
        out     USARTD
        jmp     return

;;; Print out newline
;;; @clobber 0 A
newline:
        li      H'0D'
        lr      0, A
        pi      call
        da      putchar
        li      H'0A'
        lr      0, A
        br      putchar

;;; Print out space
;;; @clobber 0 A
putspace:
        li      C' '
        lr      0, A
        br      putchar

;;; Print unsigned 16-bit integer as decimal
;;; @param 0:1 value
;;; @clobber 0 1 4 5 6 7 A
print_uint16:
        lr      A, 0
        oi      0
        bnz     print_uint16_inner
        lr      A, 1
        oi      0
        bz      print_uint16_zero
print_uint16_inner:
        lr      A, 0
        lr      4, A
        lr      A, 1
        lr      5, A            ; 4:5=value
print_uint16_loop:
        lr      A, 4
        oi      0
        bnz     print_uint16_digit
        lr      A, 5
        oi      0
        bnz     print_uint16_digit
        jmp     return
print_uint16_digit:
        clr
        lr      6, A
        li      10
        lr      7, A            ; 6:7=10
        pi      call
        da      udiv16          ; 4:5/6:7=4:5...6:7
        lr      A, 7
        lr      0, A
        pi      push0           ; push reminder
        pi      call
        da      print_uint16_loop
        pi      pop0
print_uint16_zero:
        li      C'0'
        as      0
        lr      0, A
        jmp     putchar

;;; Print signed 16-bit integer as decimal
;;; @param 0:1 value
;;; @clobber 0 1 4 5 6 7 A
print_int16:
        lr      A, 0
        oi      0
        bp      print_uint16
        pi      push0
        li      C'-'
        lr      0, A
        pi      call
        da      putchar         ; print '-'
        pi      pop0
        lr      A, 0
        com
        lr      0, A
        lr      A, 1
        com
        inc
        lr      1, A
        lr      A, 0
        lnk
        lr      0, A            ; 0:1=-value
        br      print_uint16

;;; Print out expression "operand1 operator operand2"
;;; @param @2: operand1
;;; @param @3: operand2
;;; @param 0: operator letter
;;; @clobber 0 1 A
expr:
        pi      push0           ; save operator letter
        lr      A, 2
        lr      IS, A
        lr      A, I
        lr      0, A
        lr      A, S
        lr      1, A
        pi      call
        da      print_int16
        pi      call
        da      putspace
        pi      pop0
        pi      call
        da      putchar
        pi      call
        da      putspace
        lr      A, 3
        lr      IS, A
        lr      A, I
        lr      0, A
        lr      A, S
        lr      1, A
        br      print_int16

;;; Print out answer "= result\n"
;;; @param @2: answer
;;; @clobber 0 1 A
answer:
        pi      call
        da      putspace
        li      C'='
        lr      0, A
        pi      call
        da      putchar
        pi      call
        da      putspace
        lr      A, 2
        lr      IS, A
        lr      A, I
        lr      0, A
        lr      A, S
        lr      1, A
        pi      call
        da      print_int16
        jmp     newline

;;; Scratchpad
vA:     equ     H'10'
vB:     equ     H'12'

;;; Compare 2 integers and print out "operand1 <=> operand2\n"
;;; @param @2: operand1
;;; @param @3: operand2
;;; @clobber 0 1 A
comp:
        pi      call
        da      cmpsi2
        bz      comp_eq
        bp      comp_gt
        bm      comp_lt
        li      C'?'
        br      comp_out
comp_gt:
        li      C'>'
        br      comp_out
comp_eq:
        li      C'='
        br      comp_out
comp_lt:
        li      C'<'
comp_out:
        lr      0, A
        pi      call
        da      expr
        jmp     newline

;;; Store DC to scratchpad pointed by 2
;;; @param DC value
;;; @param 2 scratchpad number
;;; @clobber H A
store_2:
        lr      A, 2
        lr      IS, A
        lr      H, DC
        lr      A, HU
        lr      I, A
        lr      A, HL
        lr      S, A
        pop

;;; Store DC to scratchpad pointed by 3
;;; @param DC value
;;; @param 3 scratchpad number
;;; @clobber H A
store_3:
        lr      A, 3
        lr      IS, A
        lr      H, DC
        lr      A, HU
        lr      I, A
        lr      A, HL
        lr      S, A
        pop

        org     H'1000'
arith:
        li      vA
        lr      2, A            ; 2 points vA
        li      vB
        lr      3, A            ; 3 points vB

        dci     0
        pi      store_2         ; vA=0
        dci     -28000
        pi      store_3         ; vB=-28000
        li      C'-'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      negsi2
        pi      call
        da      answer          ; 28000

        dci     0
        pi      store_2         ; vA=0
        dci     28000
        pi      store_3         ; vB=28000]
        li      C'-'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      negsi2
        pi      call
        da      answer          ; -28000

        dci     18000
        pi      store_2         ; vA=18000
        dci     28000
        pi      store_3         ; vB=28000
        li      C'+'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      addsi2
        pi      call
        da      answer          ; -19536

        dci     18000
        pi      store_2         ; vA=18000
        dci     -18000
        pi      store_3         ; vB=-18000
        li      C'+'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      addsi2
        pi      call
        da      answer          ; 0

        dci     -18000
        pi      store_2         ; vA=-18000
        dci     -18000
        pi      store_3         ; vB=-18000
        li      C'+'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      addsi2
        pi      call
        da      answer          ; 29536

        dci     18000
        pi      store_2         ; vA=18000
        dci     -28000
        pi      store_3         ; vB=-28000
        li      C'-'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      subsi2
        pi      call
        da      answer          ; -19536

        dci     18000
        pi      store_2         ; vA=18000
        dci     -18000
        pi      store_3         ; vB=-18000
        li      C'-'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      subsi2
        pi      call
        da      answer          ; 29536

        dci     -28000
        pi      store_2         ; vA=-28000
        dci     -18000
        pi      store_3         ; vB=-18000
        li      C'-'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      subsi2
        pi      call
        da      answer          ; -10000

        dci     100
        pi      store_2         ; vA=100
        dci     300
        pi      store_3         ; vB=300
        li      C'*'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      mulsi2
        pi      call
        da      answer          ; 30000

        dci     200
        pi      store_2         ; vA=200
        dci     -100
        pi      store_3         ; vB=-100
        li      C'*'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      mulsi2
        pi      call
        da      answer          ; -20000

        dci     300
        pi      store_2         ; vA=300
        dci     -200
        pi      store_3         ; vB=-200
        li      C'*'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      mulsi2
        pi      call
        da      answer          ; 5536

        dci     -100
        pi      store_2         ; vA=-100
        dci     300
        pi      store_3         ; vB=300
        li      C'*'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      mulsi2
        pi      call
        da      answer          ; -30000

        dci     -200
        pi      store_2         ; vA=-200
        dci     -100
        pi      store_3         ; vB=-100
        li      C'*'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      mulsi2
        pi      call
        da      answer          ; 20000

        dci     30000
        pi      store_2         ; vA=30000
        dci     100
        pi      store_3         ; vB=100
        li      C'/'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      divsi2
        pi      call
        da      answer          ; 30

        dci     -200
        pi      store_2         ; vA=-200
        dci     100
        pi      store_3         ; vB=100
        li      C'/'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      divsi2
        pi      call
        da      answer          ; -2

        dci     -30000
        pi      store_2         ; vA=-30000
        dci     -200
        pi      store_3         ; vB=-200
        li      C'/'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      divsi2
        pi      call
        da      answer          ; 150

        dci     -30000
        pi      store_2         ; vA=-30000
        dci     78
        pi      store_3         ; vB=78
        li      C'/'
        lr      0, A
        pi      call
        da      expr
        pi      call
        da      divsi2
        pi      call
        da      answer          ; -384

        dci     5000
        pi      store_2         ; vA=5000
        dci     4000
        pi      store_3         ; vB=4000
        pi      call
        da      comp

        dci     5000
        pi      store_2         ; vA=5000
        dci     5000
        pi      store_3         ; vB=5000
        pi      call
        da      comp

        dci     4000
        pi      store_2         ; vA=4000
        dci     5000
        pi      store_3         ; vB=5000
        pi      call
        da      comp           

        dci     -5000
        pi      store_2         ; vA=-5000
        dci     -4000
        pi      store_3         ; vB=-4000
        pi      call
        da      comp

        dci     -5000
        pi      store_2         ; vA=-5000
        dci     -5000
        pi      store_3         ; vB=-5000
        pi      call
        da      comp

        dci     -4000
        pi      store_2         ; vA=-4000
        dci     -5000
        pi      store_3         ; vB=-5000
        pi      call
        da      comp           

        dci     32700
        pi      store_2         ; vA=32700
        dci     32600
        pi      store_3         ; vB=32600
        pi      call
        da      comp           

        dci     32700
        pi      store_2         ; vA=32700
        dci     32700
        pi      store_3         ; vB=32700
        pi      call
        da      comp           

        dci     32600
        pi      store_2         ; vA=32600
        dci     32700
        pi      store_3         ; vB=32700
        pi      call
        da      comp           

        dci     -32700
        pi      store_2         ; vA=-32700
        dci     -32600
        pi      store_3         ; vB=-32600
        pi      call
        da      comp           

        dci     -32700
        pi      store_2         ; vA=-32700
        dci     -32700
        pi      store_3         ; vB=-32700
        pi      call
        da      comp           

        dci     -32600
        pi      store_2         ; vA=-32600
        dci     -32700
        pi      store_3         ; vB=-32700
        pi      call
        da      comp           

        dci     18000
        pi      store_2         ; vA=18000
        dci     -28000
        pi      store_3         ; vB=-28000
        pi      call
        da      comp           

        dci     18000
        pi      store_2         ; vA=18000
        dci     18000
        pi      store_3         ; vB=18000
        pi      call
        da      comp           

        dci     -28000
        pi      store_2         ; vA=-28000
        dci     18000
        pi      store_3         ; vB=18000
        pi      call
        da      comp           

        jmp     return

        include "arith.inc"

        end
