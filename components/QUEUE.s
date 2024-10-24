.section .text
*****************************************************************
** Queues
*****************************************************************
INIT:
	movem.l	%a1, -(%sp)
	lea.l	top_0, %a1		/*top address is a1*/
	move.l	%a1, in_0
	move.l	%a1, out_0
	move.w	#0, s_0

	lea.l	top_1, %a1		/*tomove.w %sr, -(%sp)p address is a1*/
	move.l	%a1, in_1
	move.l	%a1, out_1
	move.w	#0, s_1
	movem.l	(%sp)+, %a1
	rts

INPUT_QUEUE:
	movem.l	%d3-%d5/%a1-%a6,-(%sp)
	move.w	%SR, %d5
	move.w #0x2700, %SR	/*runlevel->7*/
	
	/*check the no of the que*/
	cmpi.b	#0, %d0
	bne	INPUT_Q1
	bra	INPUT_Q0

INPUT_Q0:				/*routine for the Queue no.0*/
	lea.l	top_0, %a1		/*top address is a1*/
	lea.l	bottom_0, %a2		/*bottom address is a2*/
	move.l 	in_0, %a3		/*pointer in->a3*/
	move.l	out_0, %a4		/*pointer out->a4*/
	move.w	s_0, %d3		/*s->d3*/
	jsr	INQ

	/*after the subroutine update the varibles*/
	move.w	%d3, s_0
	move.l	%a3, in_0
	move.w	%d5, %SR
	movem.l	(%sp)+,%d3-%d5/%a1-%a6
	rts
	
INPUT_Q1:
	lea.l	top_1, %a1		/*top address is a1*/
	lea.l	bottom_1, %a2		/*bottom address is a2*/
	move.l 	in_1, %a3		/*pointer in->a3*/
	move.l	out_1, %a4		/*pointer out->a4*/
	move.w	s_1, %d3		/*s->d3*/
	jsr	INQ

	/*after the subroutine update the varibles*/
	move.w	%d3, s_1
	move.l	%a3, in_1
	move.w	%d5, %SR
	movem.l	(%sp)+,%d3-%d5/%a1-%a6
	rts

INQ:
	cmp.w	#256, %d3
	bne	INQ_SUCC		/*if s not equals to 256*/
	bra	INQ_FAIL		/*if s==256*/

INQ_SUCC:
	move.b 	%d1, (%a3)

	cmp	%a2, %a3
	beq	INQ_BACK		/*reach the bottom*/
	bra	INQ_NEXT

INQ_NEXT:
	addq	#1, %a3
	addi.w	#1, %d3
	move.l	#1, %d0
	rts

INQ_BACK:
	addi.w	#1, %d3
	move.l	%a1, %a3
	move.l	#1, %d0
	rts	

INQ_FAIL:
	move.l	#0, %d0
	rts
	



OUTPUT_QUEUE:
	movem.l	%d3-%d5/%a1-%a6,-(%sp)
	move.w	%SR, %d5
	move.w #0x2700, %SR	/*runlevel->7*/
	/*check the no of the que*/
	cmpi.b	#0, %d0
	bne	OUTPUT_Q1
	bra	OUTPUT_Q0
	
OUTPUT_Q0:
	lea.l	top_0, %a1		/*top address is a1*/
	lea.l	bottom_0, %a2		/*bottom address is a2*/
	move.l	out_0, %a4		/*pointer out<-a4*/
	move.w	s_0, %d3
	jsr	OUTQ
	
	/*after the subroutine update the varibles*/
	move.w	%d3, s_0
	move.l	%a4, out_0
	
	move.w	%d5, %SR
	movem.l	(%sp)+,%d3-%d5/%a1-%a6
	rts

OUTPUT_Q1:
	lea.l	top_1, %a1		/*top address is a1*/
	lea.l	bottom_1, %a2		/*bottom address is a2*/
	move.l	out_1, %a4		/*pointer out<-a4*/
	move.w	s_1, %d3
	jsr	OUTQ

	/*after the subroutine update the varibles*/
	move.w	%d3, s_1
	move.l	%a4, out_1
	
	move.w	%d5, %SR
	movem.l	(%sp)+,%d3-%d5/%a1-%a6
	rts

OUTQ:
	cmp.w 	#0, %d3
	bne 	OUTQ_SUCC
	bra	OUTQ_FAIL

OUTQ_SUCC:
	move.b	(%a4), %d1

	cmp	%a2, %a4
	beq	OUTQ_BACK		/*reach the bottom*/
	bra	OUTQ_NEXT

OUTQ_NEXT:
	addq	#1, %a4
	subi.w	#1, %d3	
	move.l	#1, %d0
	rts

OUTQ_BACK:
	move.l	%a1, %a4
	subi.w	#1, %d3
	move.l	#1, %d0
	rts	

OUTQ_FAIL:
	move.l	#0, %d0
	rts


.section .data
	.equ	SIZE_of_QUEUE,	256

.section .bss
top_0:		.ds.b	SIZE_of_QUEUE-1
bottom_0:	.ds.b	1
in_0:		.ds.l	1
out_0:		.ds.l	1
s_0:		.ds.w	1

top_1:		.ds.b	SIZE_of_QUEUE-1
bottom_1:	.ds.b	1
s_1:		.ds.w	1
in_1:		.ds.l	1
out_1:		.ds.l	1

.end
