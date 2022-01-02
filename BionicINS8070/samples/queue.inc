;;; [queue] queue structure
queue_len:      equ     0       ; queue length
queue_size:     equ     1       ; buffer size
queue_put:      equ     2       ; queue put index
queue_get:      equ     3       ; queue get index
queue_buf:      equ     4       ; buffer start offset

;;; [queue] Initialize queue
;;; @param P2 queue work space pointer
;;; @param A queue work space size
;;; @clobber A
queue_init:
        ld      e, a
        ld      a, =0
        st      a, queue_len, p2
        st      a, queue_put, p2
        st      a, queue_get, p2
        xch     a, e            ; E=0, A=space size
        sub     a, =queue_buf
        st      a, queue_size, p2
        push    a               ; 0,SP=counter
        ld      a, @queue_buf, p2
queue_init_loop:
        ld      a, e            ; E=0
        st      a, @1, p2
        dld     a, 0, sp
        bnz     queue_init_loop
        pop     a               ; discard counter
        ret

;;; [add_p2_a] Add A to P2
;;; @param P2
;;; @param A
;;; @return P3=P2+A
add_p2_a:
        push    ea              ; save EA
        xch     a, e
        ld      a, =0
        xch     a, e
        push    ea              ; 0:E
        ld      ea, p2
        add     ea, 0, sp       ; EA=P2+E
        ld      p3, ea
        pop     ea
        pop     ea              ; restore EA
        ret

;;; [queue] Add an element to queue
;;; @param P2 queue work space pointer
;;; @param A an element
;;; @return E an element
;;; @return A 0 if queue is full
queue_add:
        push    p3
        ld      e, a            ; save element in E
        ld      a, queue_len, p2
        sub     a, queue_size, p2
        bz      queue_add_return ; A=0
        ld      a, queue_put, p2 ; 8 bits offset
        jsr     add_p2_a
        ld      a, e
        st      a, queue_buf, p3 ; store an element
        ild     a, queue_len, p2
        ild     a, queue_put, p2
        sub     a, queue_size, p2
        bnz     queue_add_return ; A is not zero
        st      a, queue_put, p2
        ld      a, =1           ; A is not zero
queue_add_return:
        pop     p3
        ret

;;; [queue] Remove an element from queue
;;; @param P2 queue work space pointer
;;; @return E an element
;;; @return A 0 if queue is empty
queue_remove:
        push    p3
        ld      a, queue_len, p2
        bz      queue_remove_return ; A is zero
queue_remove_elem
        ld      a, queue_get, p2 ; 8 bits offset
        jsr     add_p2_a
        ld      a, queue_buf, p3 ; read an element
        ld      e, a
        dld     a, queue_len, p2
        ild     a, queue_get, p2
        sub     a, queue_size, p2
        bnz     queue_remove_return ; A is not zero
        st      a, queue_get, p2 ; A is zero
        ld      a, =1            ; A is not zero
queue_remove_return:
        pop     p3
        ret

;;; Local Variables:
;;; mode: asm
;;; End:
;;; vim: set ft=asm et ts=4 sw=4:
