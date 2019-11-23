        .cr     6809
        .list   toff
        .lf     echoir.lst
        .tf     echoir.s19,s19

;;; MC6850
;;; Asynchronous Communication Interface Adapter
ACIA:   .equ    $FFC0
        ;; Control register
ACIA_C: .equ    ACIA+0
        ;; Counter Divider Select Bits
CDS_0:  .equ    0x00      ; /1
CDS_1:  .equ    0x01      ; /16
CDS_2:  .equ    0x02      ; /64
CDS_3:  .equ    0x03      ; Master Reset
        ;; Word Select Bits
WSB_0:  .equ    0x00      ; 7 Bits + Even Parity + 2 Stop Bits
WSB_1:  .equ    0x04      ; 7 bits + Odd Parity  + 2 Stop Bits
WSB_2:  .equ    0x08      ; 7 bits + Even Parity + 1 Stop Bits
WSB_3:  .equ    0x0C      ; 7 bits + Odd Parity  + 1 Stop Bits
WSB_4:  .equ    0x10      ; 8 bits + No Parity   + 2 Stop Bits
WSB_5:  .equ    0x14      ; 8 bits + No Parity   + 1 Stop Bits
WSB_6:  .equ    0x18      ; 8 bits + Even Parity + 1 Stop Bits
WSB_7:  .equ    0x1C      ; 8 bits + Odd Parity  + 1 Stop Bits
        ;; Transmit Control Bits
TCB_0:  .equ    0x00            ; RTS=Low,  Interrupt Disabled
TCB_1:  .equ    0x20            ; RTS=Low,  Interrupt Enabled
TCB_2:  .equ    0x40            ; RTS=High, Interrupt Disabled
TCB_3:  .equ    0x60            ; RTS=Low,  Interrupt Disabled
                                ; Transmit Break Level
RIEB:   .equ    0x80            ; Receive Interrupt Enable Bit.
        ;; Status register
ACIA_S: .equ    ACIA+0
RDRF:   .equ    0x01            ; Receive Data Register Full
TDRE:   .equ    0x02            ; Transmit Data Register Empty
DCDF:   .equ    0x04            ; Data Carrier Detect Flag
CTSF:   .equ    0x08            ; Clear To Send Flag
FERR:   .equ    0x10            ; Frame Error Flag
OVRN:   .equ    0x20            ; Receiver Overrun Flag
PERR:   .equ    0x40            ; Parity Error Flag
IRQF:   .equ    0x80            ; Interrupt Request Flag
        ;; Data register
ACIA_D: .equ    ACIA+1          ; Data register

        .sm     ram
        .org    $2000

rx_queue_size:  .equ    128
rx_queue:       .bs     rx_queue_size

        .no     $F000
stack:  .equ    *

        .sm     code
        .org    $1000
initialize:
        lds     #stack
        ldx     #rx_queue
        ldb     #rx_queue_size
        lbsr    queue_init
        ;; initialize ACIA
        lda     #CDS_1|CDS_0    ; master reset
        sta     ACIA_C
        lda     #WSB_5          ; 8 Bits + No parity + 1 Stop bits
        ora     #RIEB           ; enable Rx interruputs
        sta     ACIA_C
        cli                     ; Clear IRQ mask

        ldx     #rx_queue
receive_loop:
        sei                     ; Set IRQ mask
        lbsr    queue_remove
        cli                     ; Clear IRQ mask
        bcc     receive_loop
transmit_loop:
        ldb     ACIA_S
        bitb    #TDRE
        beq     transmit_loop
transmit_data:
        sta     ACIA_D
        bra     receive_loop

;;; [queue] queue structure
queue_len:      .equ    0       ; queue length
queue_size:     .equ    1       ; buffer size
queue_put:      .equ    2       ; queue put index
queue_get:      .equ    3       ; queue get index
queue_buf:      .equ    4       ; buffer start offset
;;; [queue] Initialize queue
;;; @param X queue work space pointer
;;; @param B queue work space size
;;; @clobber B
queue_init:
        clr     queue_len,x
        clr     queue_put,x
        clr     queue_get,x
        subb    #queue_buf
        stb     queue_size,x
        rts
;;; [queue] Add an element to queue
;;; @param X queue work space pointer
;;; @param A an element
;;; @return CC.C 0 if queue is full
queue_add:
        pshs    u,b
        ldb     queue_len,x
        cmpb    queue_size,x
        blo     queue_add_elem
        puls    b,u,pc          ; carry is cleared
queue_add_elem:
	pshs    a
        clra
        ldb     queue_put,x     ; 16 bits offset
        leau    queue_buf,x
        leau    d,u
        lda     ,s              ; an element
        sta     ,u
        inc     queue_len,x
        incb
        stb     queue_put,x
        cmpb    queue_size,x
        blo     queue_add_return ; carry is set
        clr     queue_put,x
        orcc    #1              ; set carry
queue_add_return:
	puls    a,b,u,pc
;;; [queue] Remove an element from queue
;;; @param X queue work space pointer
;;; @return A an element
;;; @return CC.C 0 if queue is empty
queue_remove:
        tst     queue_len,x
        bne     queue_remove_elem
        andcc   #~1             ; clear carry
        rts
queue_remove_elem:
        pshs    u,b
        clra
        ldb     queue_get,x     ; 16 bits offset
        leau    queue_buf,x
        lda     d,u
        dec     queue_len,x
        incb
        stb     queue_get,x
        cmpb    queue_size,x
        blo     queue_remove_return ; carry is set
        clr     queue_get,x
        orcc    #1              ; set carry
queue_remove_return:
        puls    b,u,pc

isr_irq:
        lda     ACIA_S
        bita    #IRQF
        beq     isr_irq_return
isr_irq_receive:
        bita    #RDRF
        beq     isr_irq_recv_end
        lda     ACIA_D
        ldx     #rx_queue
        lbsr    queue_add
isr_irq_recv_end:
isr_irq_return:
        rti

        .no     $FFF8
        .dw     isr_irq

        .no     $FFFE
        .dw     initialize
