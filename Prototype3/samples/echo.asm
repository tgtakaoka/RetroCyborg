	.cr	6809
	.list	toff
	.lf	echo.lst
	.tf	echo.s19,s19

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   .equ	$FFC0
        .inc    mc6850.inc

	.sm	ram
	.org	$F000
stack:	.equ	*

	.sm	code
	.org	$1000
initialize:
	lds	#stack
	lda	#CDS_RESET_gc   ; Master reset
	sta	ACIA_control
	lda	#WSB_8N1_gc        ; 8 bits + No Parity + 1 Stop Bits
	ora	#TCB_EI_gc|RIEB_bm ; Transmit, Receive interrupts enable
	sta	ACIA_control
	sei			; Set Interrupt mask

receive_loop:
	lda	ACIA_status
	bita	#RDRF_bm
	beq	receive_loop
receive_data:
	ldb	ACIA_data
transmit_loop:
	lda	ACIA_status
	bita	#TDRE_bm
	beq	transmit_loop
transmit_data:
	stb	ACIA_data
        cmpb    #$0d
        bne     receive_loop
        ldb     #$0a
	bra	transmit_loop

	.no	$FFFE
	.dw	initialize
