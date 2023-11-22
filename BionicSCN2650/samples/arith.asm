;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     2650
        include "scn2650.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USARTD: equ     H'00'           ; Data register
USARTS: equ     H'01'           ; Status register
USARTC: equ     H'01'           ; Control register
        include "i8251.inc"

        org     ORG_RESET
        ppsu    PSU_II          ; disable interrupt
        ppsl    PSL_WC+PSL_COM  ; with carry, logical compare
init_usart:
        eorz    r0              ; clear R0
        wrte,r0 USARTC
        wrte,r0 USARTC
        wrte,r0 USARTC          ; safest way to sync mode
        lodi,r0 CMD_IR_bm
        wrte,r0 USARTC          ; reset
        nop
        nop
        lodi,r0 MODE_STOP1_gc + MODE_LEN8_gc + MODE_BAUD_X16
        wrte,r0 USARTC          ; async 1stop 8data x16
        nop
        nop
        lodi,r0 CMD_RTS_bm + CMD_DTR_bm + CMD_ER_bm + CMD_RxEN_bm + CMD_TxEN_bm
        wrte,r0 USARTC    ; RTS/DTR, error reset, Rx enable, Tx enable

        bcta,un arith

;;; Print out char
;;; @param R0 char
;;; @clobber R0
putchar:
        strr,r1 putchar_r1
putchar_loop:
        rede,r1 USARTS
        tmi,r1  ST_TxRDY_bm
        bcfr,eq putchar_loop
        wrte,r0 USARTD
        comi,r0 H'0D'
        bctr,eq putchar_nl
        lodr,r1 putchar_r1
        retc,un
putchar_nl:
        lodi,r0 H'0A'
        bctr,un putchar_loop
putchar_r1:
        res     1

newline:
        lodi,r0 H'0D'
        bctr,un putchar

putspace:
        lodi,r0 A' '
        bctr,un putchar

;;; Print unsigned 16-bit integer as decimal
;;; @param R0:R1: value
;;; @clobber R0 R1
print_uint16:
        strr,r2 print_uint16_r2
        lodi,r2 0
        strr,r2 print_uint16_buf
print_uint16_inner:
        stra,r0 udiv16_divident
        stra,r1 udiv16_divident+1
print_uint16_loop:
        eorz    r0
        stra,r0 udiv16_divisor
        lodi,r0 10
        stra,r0 udiv16_divisor+1
        bsta,un udiv16            ; R0:R1=value / 10
        stra,r0 udiv16_divident
        loda,r0 udiv16_divident+1 ; R0=value % 10
        stra,r1 udiv16_divident+1
        iori,r0 A'0'
        stra,r0 print_uint16_buf,r2,+ ; buf[++R2]=R0+'0'
        lodz    r1
        iora,r0 udiv16_divident
        bcfr,eq print_uint16_loop
        cpsl    PSL_C           ; PSL.C=0
        addi,r2 1
print_uint16_out:
        loda,r0 print_uint16_buf,r2,-
        bctr,eq print_uint16_end
        bsta,un putchar
        bctr,un print_uint16_out
print_uint16_end:
        lodr,r2 print_uint16_r2
        retc,un
print_uint16_r2:
        res     1
print_uint16_buf:
        res     8

;;; Print signed 16-bit integer as decimal
;;; @param R0:R1 value
;;; @clobber R0 R1
print_int16:
        iorz    r0
        bcfa,lt print_uint16    ; branch if R0:R1>=0
        bsta,un negsi1          ; R0:R1=-value
        strr,r0 print_uint16_buf
        lodi,r0 A'-'
        bsta,un putchar
        lodr,r0 print_uint16_buf
        bcta,un print_uint16

expr_r0:
        res     1
expr:
        strr,r0 expr_r0
        loda,r0 arith_work+1,r2
        strz    r1
        loda,r0 arith_work,r2
        stra,r0 arith_work
        stra,r1 arith_work+1
        bstr,un print_int16
        bsta,un putspace
        lodi,r2 arith_work-arith_work
        lodr,r0 expr_r0
        bsta,un putchar
        bsta,un putspace
        loda,r0 arith_work+1,r3
        strz    r1
        loda,r0 arith_work,r3
        bctr,un print_int16

answer:
        bsta,un putspace
        lodi,r0 A'='
        bsta,un putchar
        bsta,un putspace
        loda,r0 arith_work+1,r2
        strz    r1
        loda,r0 arith_work,r2
        bsta,un print_int16
        bcta,un newline

comp:
        bsta,un cmpsi2
        bctr,gt comp_gt
        bctr,eq comp_eq
        bctr,lt comp_lt
        lodi,r0 A'?'
        bctr,un comp_out
comp_gt:
        lodi,r0 A'>'
        bctr,un comp_out
comp_eq:
        lodi,r0 A'='
        bctr,un comp_out
comp_lt:
        lodi,r0 A'<'
comp_out:
        bsta,un expr
        bcta,un newline

        org     H'0F00'
arith_work:     equ     $
        res     2
p18000: acon    18000
p28000: acon    28000
n18000: acon    -18000
n28000: acon    -28000
p300:   acon    300
p200:   acon    200
n200:   acon    -200
n100:   acon    -100
p100:   acon    100
n300:   acon    -300
zero:   acon    0
p78     acon    78
p30000: acon    30000
n30000: acon    -30000
p5000:  acon    5000
p4000:  acon    4000
n5000:  acon    -5000
n4000:  acon    -4000
p32700: acon    32700
p32600: acon    32600
n32700: acon    -32700
n32600: acon    -32600

        org     H'1000'
arith:
        lodi,r2 zero-arith_work
        lodi,r3 n28000-arith_work
        lodi,r0 A'-'
        bsta,un expr
        bsta,un negsi2
        bsta,un answer          ; 28000

        lodi,r2 zero-arith_work
        lodi,r3 p28000-arith_work
        lodi,r0 A'-'
        bsta,un expr
        bsta,un negsi2
        bsta,un answer          ; -28000

        lodi,r2 p18000-arith_work
        lodi,r3 p28000-arith_work
        lodi,r0 A'+'
        bsta,un expr
        bsta,un addsi2
        bsta,un answer          ; -19536

        lodi,r2 p18000-arith_work
        lodi,r3 n18000-arith_work
        lodi,r0 A'+'
        bsta,un expr
        bsta,un addsi2
        bsta,un answer          ; 0

        lodi,r2 n18000-arith_work
        lodi,r3 n18000-arith_work
        lodi,r0 A'+'
        bsta,un expr
        bsta,un addsi2
        bsta,un answer          ; 29536

        lodi,r2 p18000-arith_work
        lodi,r3 n28000-arith_work
        lodi,r0 A'-'
        bsta,un expr
        bsta,un subsi2
        bsta,un answer          ; -19536

        lodi,r2 p18000-arith_work
        lodi,r3 n18000-arith_work
        lodi,r0 A'-'
        bsta,un expr
        bsta,un subsi2
        bsta,un answer          ; 29536

        lodi,r2 n28000-arith_work
        lodi,r3 n18000-arith_work
        lodi,r0 A'-'
        bsta,un expr
        bsta,un subsi2
        bsta,un answer          ; -10000

        lodi,r2 p100-arith_work
        lodi,r3 p300-arith_work
        lodi,r0 A'*'
        bsta,un expr
        bsta,un umulsi2
        bsta,un answer          ; 30000

        lodi,r2 p200-arith_work
        lodi,r3 p100-arith_work
        lodi,r0 A'*'
        bsta,un expr
        bsta,un umulsi2
        bsta,un answer          ; 20000

        lodi,r2 p300-arith_work
        lodi,r3 n200-arith_work
        lodi,r0 A'*'
        bsta,un expr
        bsta,un mulsi2
        bsta,un answer          ; 5536

        lodi,r2 p100-arith_work
        lodi,r3 n300-arith_work
        lodi,r0 A'*'
        bsta,un expr
        bsta,un mulsi2
        bsta,un answer          ; -30000

        lodi,r2 n200-arith_work
        lodi,r3 n100-arith_work
        lodi,r0 A'*'
        bsta,un expr
        bsta,un mulsi2
        bsta,un answer          ; 20000

        lodi,r2 p30000-arith_work
        lodi,r3 p100-arith_work
        lodi,r0 A'/'
        bsta,un expr
        bsta,un udivsi2
        bsta,un answer          ; 30

        lodi,r2 n200-arith_work
        lodi,r3 p100-arith_work
        lodi,r0 A'/'
        bsta,un expr
        bsta,un divsi2
        bsta,un answer          ; -2

        lodi,r2 n30000-arith_work
        lodi,r3 n200-arith_work
        lodi,r0 A'/'
        bsta,un expr
        bsta,un divsi2
        bsta,un answer          ; 150

        lodi,r2 n30000-arith_work
        lodi,r3 p78-arith_work
        lodi,r0 A'/'
        bsta,un expr
        bsta,un divsi2
        bsta,un answer          ; -384

        lodi,r2 p5000-arith_work
        lodi,r3 p4000-arith_work
        bsta,un comp

        lodi,r2 p5000-arith_work
        lodi,r3 p5000-arith_work
        bsta,un comp

        lodi,r2 p4000-arith_work
        lodi,r3 p5000-arith_work
        bsta,un comp

        lodi,r2 n5000-arith_work
        lodi,r3 n4000-arith_work
        bsta,un comp

        lodi,r2 n5000-arith_work
        lodi,r3 n5000-arith_work
        bsta,un comp

        lodi,r2 n4000-arith_work
        lodi,r3 n5000-arith_work
        bsta,un comp

        lodi,r2 p32700-arith_work
        lodi,r3 p32600-arith_work
        bsta,un comp

        lodi,r2 p32700-arith_work
        lodi,r3 p32700-arith_work
        bsta,un comp

        lodi,r2 p32600-arith_work
        lodi,r3 p32700-arith_work
        bsta,un comp

        lodi,r2 n32700-arith_work
        lodi,r3 n32600-arith_work
        bsta,un comp

        lodi,r2 n32700-arith_work
        lodi,r3 n32700-arith_work
        bsta,un comp

        lodi,r2 n32600-arith_work
        lodi,r3 n32700-arith_work
        bsta,un comp

        lodi,r2 p18000-arith_work
        lodi,r3 n28000-arith_work
        bsta,un comp

        lodi,r2 p18000-arith_work
        lodi,r3 p18000-arith_work
        bsta,un comp

        lodi,r2 n28000-arith_work
        lodi,r3 p18000-arith_work
        bsta,un comp

        halt

        include "arith.inc"

        end
