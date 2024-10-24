	.section .text

INTERPUT:
	**i dont know how can i change the running level :(

	****************************
	**input
	**d1 : ch
	****************************

	cmpi	#0, %d1
	beq	inter_put	/*ch!=0ならば，何もせずに復帰 */
	rts			/*if its 0 jump back*/
inter_put:
	movem.l	%d0-%d1/%a1, -(%sp)
	moveq	#1, %d0		/*to use Queue for transmission*/
	jsr	OUTPUT_QUEUE	/*d0:success of fail, d1:que's output*/

	cmpi	#0, %d0
	bne	inter_put2
	**送信割り込みをマスク(USTCNT1を操作)
	andi	#0xfff8, USTCNT1	/*if outq failed*/
	rts

inter_put2:
	move.l 	UTX1, %d2
	cmpi	#0, s_1
	bne	not_empty
	add.b	#0x8800, %d2
	bra	inter_put3
not_empty:	
	add.b	#0x0800, %d2

inter_put3:
	add.b	%d1, %d2	/*d2<-d1+d2*/
	move.w	%d2, UTX1
	movem.l	(%sp)+, %d0-%d1/%a1
	rts
	
