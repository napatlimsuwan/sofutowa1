.include	"./components/QUEUE.s"

.section .text
INTERGET:
	movem.l	%d1, -(%sp)
	cmp	#0, %d1
	beq	INTERGET_INQ
	movem.l	(%sp)+, %d1
	rts

INTERGET_INQ:
	movem.l	%d0, -(%sp)
	move.l	#0, %d0
	jsr	INPUT_QUEUE
	movem.l	(%sp)+, %d0
	rts
.end
