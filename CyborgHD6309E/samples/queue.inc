;;; -*- mode: asm; mode: flying-spell; -*-

;;; [queue] queue structure
queue_len:	equ	0	; queue length
queue_size:	equ	1	; buffer size
queue_put:	equ	2	; queue put index
queue_get:	equ	3	; queue get index
queue_buf:	equ	4	; buffer start offset

;;; [queue] Initialize queue
;;; @param X queue work space pointer
;;; @param B queue work space size
;;; @clobber B
queue_init:
	pshs	x,d
        clra
.clear_loop:
	clr	,x+
	subd	#1
	bne	.clear_loop
	puls	d,x
	subb	#queue_buf
	stb	queue_size,x
	rts

;;; [queue] Add an element to queue
;;; @param X queue work space pointer
;;; @param A an element
;;; @return CC.C 0 if queue is full
queue_add:
	pshs	u,b,a
	ldb	queue_len,x
	cmpb	queue_size,x
	blo	.add_elem
	puls	a,b,u,pc        ; carry is cleared
.add_elem:
	clra
	ldb	queue_put,x	; 16 bits offset
	leau	queue_buf,x
	leau	d,u
	lda	,s		; an element
	sta	,u
	inc	queue_len,x
	incb
	stb	queue_put,x
	cmpb	queue_size,x
	blo	.return         ; carry is set
	clr	queue_put,x
	orcc	#1		; set carry
.return:
	puls	a,b,u,pc

;;; [queue] Remove an element from queue
;;; @param X queue work space pointer
;;; @return A an element
;;; @return CC.C 0 if queue is empty
queue_remove:
	tst	queue_len,x
	bne	.remove_elem
	andcc	#~1		; clear carry
	rts
.remove_elem: 
	pshs	u,b
	clra
	ldb	queue_get,x	; 16 bits offset
	leau	queue_buf,x
	lda	d,u
	dec	queue_len,x
	incb
	stb	queue_get,x
	cmpb	queue_size,x
	blo	.return         ; carry is set
	clr	queue_get,x
	orcc	#1		; set carry
.return:
        puls	b,u,pc
        
