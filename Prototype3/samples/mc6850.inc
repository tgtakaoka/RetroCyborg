;;; -*- mode: asm; mode: flyspell-prog; -*-

;;; MC6850
;;; Asynchronous Communication Interface Adapter

;;; Control register
ACIA_control:   .equ	ACIA+0
	;; Counter Divider Select Bits
CDS_gm:         .equ    0x03    ; Group mask
CDS_DIV1_gc:	.equ	0x00    ; /1
CDS_DIV16_gc:	.equ	0x01    ; /16
CDS_DIV64_gc:	.equ	0x02    ; /64
CDS_RESET_gc:	.equ	0x03    ; Master Reset
	;; Word Select Bits
WSB_gm:         .equ    0x1c    ; Group mask
WSB_7E2_gc:	.equ	0x00    ; 7 Bits + Even Parity + 2 Stop Bits
WSB_7O2_gc:	.equ	0x04    ; 7 bits + Odd Parity  + 2 Stop Bits
WSB_7E1_gc:	.equ	0x08    ; 7 bits + Even Parity + 1 Stop Bits
WSB_7O1_gc:	.equ	0x0C    ; 7 bits + Odd Parity  + 1 Stop Bits
WSB_8N2_gc:	.equ	0x10    ; 8 bits + No Parity   + 2 Stop Bits
WSB_8N1_gc:	.equ	0x14    ; 8 bits + No Parity   + 1 Stop Bits
WSB_8E1_gc:	.equ	0x18    ; 8 bits + Even Parity + 1 Stop Bits
WSB_8O1_gc:	.equ	0x1C    ; 8 bits + Odd Parity  + 1 Stop Bits
	;; Transmit Control Bits
TCB_gm:         .equ    0x60    ; Group mask
TCB_DI_gc:	.equ	0x00    ; RTS=Low,  Tx Interrupt Disabled
TCB_EI_gc:	.equ	0x20    ; RTS=Low,  Tx Interrupt Enabled
TCB_RTS_gc:	.equ	0x40    ; RTS=High, Tx Interrupt Disabled
TCB_BREAK_gc:	.equ	0x60    ; RTS=Low,  Tx Interrupt Disabled
				; Transmit Break Level
RIEB_bm:	.equ	0x80    ; Receive Interrupt Enable Bit mask

;;; Status register
ACIA_status:    .equ	ACIA+0
RDRF_bm:	.equ	0x01		; Receive Data Register Full
TDRE_bm:	.equ	0x02		; Transmit Data Register Empty
DCDF_bm:	.equ	0x04		; Data Carrier Detect Flag
CTSF_bm:	.equ	0x08		; Clear To Send Flag
FERR_bm:	.equ	0x10		; Frame Error Flag
OVRN_bm:	.equ	0x20		; Receiver Overrun Flag
PERR_bm:	.equ	0x40		; Parity Error Flag
IRQF_bm:	.equ	0x80		; Interrupt Request Flag

;;; Data register
ACIA_data:      .equ	ACIA+1		; Data register
