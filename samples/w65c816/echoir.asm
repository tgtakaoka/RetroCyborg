;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     65816
        .include "w65c816.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA    =       $DF00
        .include "mc6850.inc"
RX_INT_TX_NO    =       WSB_8N1_gc|RIEB_bm

        *=      NVEC_IRQ        ; native vector
        .word   isr_irq

        *=      VEC_RESET
        .word   initialize

        *=      $2000
rx_queue_size   =       128
rx_queue:
        *=      *+rx_queue_size

        *=      $1000
stack   =       *-1
initialize:
        clc
        xce                     ; native mode
        rep     #P_X            ; 16-bit index
        longi   on
        ldx     #stack
        txs
        cld                     ; clear decimal flag
        ldx     #rx_queue
        lda     #rx_queue_size
        jsr     queue_init
        ;; initialize ACIA
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #RX_INT_TX_NO
        sta     ACIA_control
        cli                     ; Enable IRQ

loop:
        jsr     getchar
        bcs     loop
        ora     #0
        beq     halt_to_system
        jsr     putchar
        cmp     #$0D
        bne     loop
        lda     #$0A
        jsr     putchar
        jmp     loop
halt_to_system:
        brk
        .byte   0               ; halt to system

getchar:
        ldx     #rx_queue
        sei                     ; Disable IRQ
        jsr     queue_remove
        cli                     ; Enable IRQ
        rts

putchar:
        pha
putchar_loop:
        lda     ACIA_status
        and     #TDRE_bm
        beq     putchar_loop
        pla
        sta     ACIA_data
        rts

        .include "queue.inc"

isr_irq:
        ;; P_D is cleared on interrupt
        sep     #P_M            ; 8-bit memory
        pha                     ; save A
        phx                     ; save X
        lda     ACIA_status
        and     #IRQF_bm
        beq     isr_irq_return
isr_irq_receive:
        lda     ACIA_status
        and     #RDRF_bm
        beq     isr_irq_return
        lda     ACIA_data
        ldx     #rx_queue
        jsr     queue_add
isr_irq_return:
        plx                     ; restore X
        pla                     ; restore A
        rti                     ; restore P and PC
