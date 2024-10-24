.include    "./components/QUEUE.s"

.section .text

PUTSTRING:
	cmpi.l	#0, %d1
	beq	PUTSTRING_INIT
	rts

PUTSTRING_INIT:
	movem.l	%d1/%d7/%a1-%a6, -(%sp)
	move.l	#0, size_put
	move.l	size_put, %d7
	move.l	%d2, ptr_put
	cmp	#0, %d3
	beq	PUTSTRING_LOOP
	
PUTSTRING_LOOP:
	cmp	%d3, %d7
	beq	PUTSTRING_UNMASK

	moveq.l	#1, %d0
	move.l	ptr_put, %a6
	move.b	(%a6), %d1
	jsr	INPUT_QUEUE
	cmp	#0, %d0
	beq	PUTSTRING_UNMASK
	
	addq.l	#1, %d7
	addq.l	#1, %a6
	move.l	%d7, size_put
	move.l	%a6, ptr_put
	bra	PUTSTRING_LOOP

PUTSTRING_UNMASK:
	ori.w	#0xe107, USTCNT1
	bra	PUTSTRING_RETURN

PUTSTRING_RETURN:
	move.l	size_put, %d0
	movem.l	(%sp)+,	%d1/%d7/%a1-%a6
	rts

.section .data
size_put:	.ds.l	1
ptr_put:	.ds.l	1

.end
