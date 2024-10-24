.include    "./components/QUEUE.s"

.section .text

GETSTRING:
	cmpi.l	#0, %d1
	beq	GETSTRING_INIT
	rts

GETSTRING_INIT:
	movem.l	%d1/%d7/%a1-%a6, -(%sp)
	move.l	#0, size_get
	move.l	size_get, %d7
	move.l	%d2, ptr_get
	move.l	ptr_get, %a6
	bra	GETSTRING_LOOP

GETSTRING_LOOP:
	cmp	%d3, %d7
	beq	GETSTRING_RETURN

	moveq.l	#0, %d0
	jsr	OUTPUT_QUEUE
	cmp	#0, %d0
	beq	GETSTRING_RETURN
	
	move.b	%d1, (%a6)
	addq.l	#1, %d7
	addq.l	#1, %a6
	move.l	%d7, size_get
	move.l	%a6, ptr_get
	bra	GETSTRING_LOOP

GETSTRING_RETURN:
	move.l	size_get, %d0
	movem.l	(%sp)+,	%d1/%d7/%a1-%a6
	rts

.section .data
size_get:	.ds.l	1
ptr_get:	.ds.l	1

.end
