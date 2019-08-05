	.cr	6809
	.list	toff
	.lf	echo.lst
	.tf	echo.s19,s19

;;; MC6850
;;; Asynchronous Communication Interface Adapter
ACIA:	.equ	$FFC0
	;; Control register
ACIA_C:	.equ	ACIA+0
	;; Counter Divider Select Bits
CDS_0:	.equ	0x00	  ; /1
CDS_1:	.equ	0x01	  ; /16
CDS_2:	.equ	0x02	  ; /64
CDS_3:	.equ	0x03	  ; Master Reset
	;; Word Select Bits
WSB_0:	.equ	0x00	  ; 7 Bits + Even Parity + 2 Stop Bits
WSB_1:	.equ	0x04      ; 7 bits + Odd Parity  + 2 Stop Bits
WSB_2:	.equ	0x08	  ; 7 bits + Even Parity + 1 Stop Bits
WSB_3:	.equ	0x0C	  ; 7 bits + Odd Parity  + 1 Stop Bits
WSB_4:	.equ	0x10	  ; 8 bits + No Parity   + 2 Stop Bits
WSB_5:	.equ	0x14	  ; 8 bits + No Parity   + 1 Stop Bits
WSB_6:	.equ	0x18	  ; 8 bits + Even Parity + 1 Stop Bits
WSB_7:	.equ	0x1C	  ; 8 bits + Odd Parity  + 1 Stop Bits
	;; Transmit Control Bits
TCB_0:	.equ	0x00		; RTS=Low,  Interrupt Disabled
TCB_1:	.equ	0x20		; RTS=Low,  Interrupt Enabled
TCB_2:	.equ	0x40		; RTS=High, Interrupt Disabled
TCB_3:	.equ	0x60		; RTS=Low,  Interrupt Disabled
				; Transmit Break Level
RIEB:	.equ	0x80		; Receive Interrupt Enable Bit.
	;; Status register
ACIA_S:	.equ	ACIA+0
RDRF:	.equ	0x01		; Receive Data Register Full
TDRE:	.equ	0x02		; Transmit Data Register Empty
DCDF:	.equ	0x04		; Data Carrier Detect Flag
CTSF:	.equ	0x08		; Clear To Send Flag
FERR:	.equ	0x10		; Frame Error Flag
OVRN:	.equ	0x20		; Receiver Overrun Flag
PERR:	.equ	0x40		; Parity Error Flag
IRQF:	.equ	0x80		; Interrupt Request Flag
	;; Data register
ACIA_D:	.equ	ACIA+1		; Data register

	.org	$1000
initialize:
	lds	#initialize
	lda	#CDS_0|WSB_5|TCB_0 ; 8 Bits, 1 Stop.
				; Interrupt disabled.
	sta	ACIA_C

receive_loop:
	lda	ACIA_S
	bita	#RDRF
	beq	receive_loop
receive_data:
	ldb	ACIA_D
transmit_loop:
	lda	ACIA_S
	bita	#TDRE
	beq	transmit_loop
transmit_data:
	stb	ACIA_D
	bra	receive_loop

	.no	$FFFE
	.dw	initialize
