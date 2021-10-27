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
queue_init_loop:
	clr	,x+
	subd	#1
	bne	queue_init_loop
	puls	d,x
	subb	#queue_buf
	stb	queue_size,x
	rts

;;; [queue] Add an element to queue
;;; @param X queue work space pointer
;;; @param A an element
;;; @return CC.C 0 if queue is full
queue_add:
	pshs	b
	ldb	queue_len,x
	cmpb	queue_size,x
	bhs     queue_add_return ;carry is cleared
	ldb	queue_put,x	; 8 bits offset
        pshs    x
        abx                     ; X+=B
	sta	queue_buf,x     ; store an element
        puls    x
	inc	queue_len,x
	incb
	stb	queue_put,x
	cmpb	queue_size,x
	blo	queue_add_return ; carry is set
	clr	queue_put,x
	orcc	#1		; set carry
queue_add_return:
	puls	b,pc

;;; [queue] Remove an element from queue
;;; @param X queue work space pointer
;;; @return A an element
;;; @return CC.C 0 if queue is empty
queue_remove:
	tst	queue_len,x
	bne	queue_remove_elem
	andcc	#~1		; clear carry
	rts
queue_remove_elem:
	pshs	b
	ldb	queue_get,x	; 8 bits offset
        pshs    x
        abx                     ; X+=B
	lda	queue_buf,x
        puls    x
	dec	queue_len,x
	incb
	stb	queue_get,x
	cmpb	queue_size,x
	blo	queue_remove_return ; carry is set
	clr	queue_get,x
	orcc	#1		; set carry
queue_remove_return:
        puls	b,pc
        
