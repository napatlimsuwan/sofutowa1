
****************************************************************
** Various Register Definition
****************************************************************
*******************************
** Head of the Register Group
*******************************
    .equ REGBASE, 0xFFF000 | DMAP is used.
    .equ IOBASE, 0x00d00000
*******************************
** Registers Related to Interrupts
*******************************
    .equ IVR, REGBASE+0x300 | Interrupt Vector Register
    .equ IMR, REGBASE+0x304 | Interrupt Mask Register
    .equ ISR, REGBASE+0x30c | Interrupt Status Register
    .equ IPR, REGBASE+0x310 | Interrupt Pending Register
*******************************
** Registers Related to the Timer
*******************************
    .equ TCTL1, REGBASE+0x600 |Timer1 Control Register
    .equ TPRER1, REGBASE+0x602 |Timer1 Prescaler Register
    .equ TCMP1, REGBASE+0x604 |Timer1 Compare Register
    .equ TCN1, REGBASE+0x608 |Timer1 Counter Register
    .equ TSTAT1, REGBASE+0x60a |Timer1 Status Register
*******************************
** Registers Related to UART1 (Transmitter and Receiver)
*******************************
    .equ USTCNT1, REGBASE+0x900 |UART1 Status / Control Register
    .equ UBAUD1, REGBASE+0x902 | UART 1 Baud Control Register
    .equ URX1, REGBASE+0x904 | UART 1 Receiver register
    .equ UTX1, REGBASE+0x906 | UART 1 Transmitter Register
*******************************
** LED
*******************************
    .equ LED7, IOBASE+0x000002f | Register for LED mounted on the board
    .equ LED6, IOBASE+0x000002d | Refer to Appendix A.4.3.1 for a way to use
    .equ LED5, IOBASE+0x000002b
    .equ LED4, IOBASE+0x0000029
    .equ LED3, IOBASE+0x000003f
    .equ LED2, IOBASE+0x000003d
    .equ LED1, IOBASE+0x000003b
    .equ LED0, IOBASE+0x0000039
****************************************************************
** Reservation of the stack region
****************************************************************
.section .bss
.even
SYS_STK:
    .ds.b   0x4000    | System stack region
    .even
SYS_STK_TOP:        | End of the system stack region
****************************************************************
** Initialization
** A specific value has been set to internal device registers.
** Refer to each register specification in Appendix B to know the above reason.
****************************************************************
.section .text
.even
boot:
* Prohibit an interrupt into the supervisor and during performing various settings.
    move.w  #0x2000, %SR		| run at lv.0
    lea.l   SYS_STK_TOP, %SP    | Set SSP
******************************
**Initialization of the interrupt controller
******************************
    move.b  #0x40, IVR                  | Set the user interrupt vector
                                        | number to 0x40+level.
    move.l  #0x00ff3ffb, IMR            | Allow UART1 interrupts
    move.l  #UART1_INTERRUPT, 0x110     |Set the interrupt subroutine for level 4 interrupt
******************************
** Initialization related to the transmitter and the receiver (UART1)
** (The interrupt level has been fixed to 4.)
******************************
    move.w  #0x0000, USTCNT1    | Reset
    move.w  #0xe100, USTCNT1    | Transmission and reception possible
                                | no parity, 1 stop, 8 bit
                                | allow only tranmission interrupt
    move.w  #0x0038, UBAUD1     | baud rate = 230400 bps
*************************
** Initialization related to the timer (The interrupt level has been fixed to 6.)
*************************
    move.w  #0x0004, TCTL1              | Restart, an interrupt impossible
                                        | Count the time with the 1/16 of the system clock
                                        | as a unit
                                        | Stop the timer use
	bra     MAIN
	
*****************************************************************
** Main program
*****************************************************************
	
.section .text
.even
MAIN :
	jsr 	INIT
	ori.l	#0x8c00, UTX1	/*init the utx1*/
	move.w	#0x2000, %SR
LOOP:	
	move.l	#0, %d1
	lea.l	WORK, %a1
	move.l	%a1, %d2
	move.l	#256, %d3
	jsr 	GETSTRING
	move.l	#0, %d1
	move.l	%d0, %d3
	lea.l	WORK, %a1
	move.l	%a1, %d2
	jsr	PUTSTRING
	bra	LOOP




*****************************************************************
** UART1 Interrupt
*****************************************************************
UART1_INTERRUPT:
	movem.l %d1-%d4, -(%sp)
	move.w  UTX1, %d1
	btst.b  #15, %d1
	bne     TRANSMITTER_INTERRUPT
    move.w	URX1, %d3
    clr.l   %d2
    move.b  %d3, %d2
    btst.b	#13, %d3
    bne     RECEIVER_INTERRUPT

TRANSMITTER_INTERRUPT:
    move.l  #0, %d1
	jsr     INTERPUT
    bra     UART1_INTERRUPT_END
RECEIVER_INTERRUPT:
    move.l  #0, %d1
    jsr     INTERPUT
**  bra     UART1_INTERRUPT_END
UART1_INTERRUPT_END:
**  move.b  #'E', LED7
**  move.b  #'n', LED6
**  move.b  #'d', LED5
    movem.l (%sp)+, %d1-%d4
    rte

*****************************************************************
** Transmitter interrupt (INTERPUT)
*****************************************************************
INTERPUT:
	movem.l	%d0-%d1/%a1, -(%sp)
    move.w  %sr, -(%sp)
	move.w  #0x2700, %sr	    /* runlevel->7 */
	cmpi	#0, %d1
	bne	    INTERPUT_END	    /* ch!=0ならば，何もせずに復帰 */
INTERPUT_START:
	moveq.l	#1, %d0		        /* to use Queue for transmission */
	jsr	    OUTPUT_QUEUE	    /* d0:success of fail, d1:que's output */

	cmpi	#0, %d0
	beq	    INTERPUT_FAIL

INTERPUT_SUCCESS:
    move.w	#0x0800, %d2
	move.b 	%d1, %d2
	move.w	%d2, UTX1
    bra     INTERPUT_END
INTERPUT_FAIL:
	andi	#0xfff8, USTCNT1	/* if outq failed */
INTERPUT_END:
    move.w 	(%sp)+, %sr
	movem.l	(%sp)+, %d0-%d1/%a1
	rts

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
	cmp.w	#1, %d3
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

GETSTRING:
	cmpi.l	#0, %d1
	beq	GETSTRING_INIT
	rts

GETSTRING_INIT:
	movem.l	%d1/%d7/%a1-%a6, -(%sp)
	move.l	#0, sz
	move.l	sz, %d7
	move.l	%d2, i
	move.l	i, %a6
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
	move.l	%d7, sz
	move.l	%a6, i
	bra	GETSTRING_LOOP

GETSTRING_RETURN:
	move.l	sz, %d0
	movem.l	(%sp)+,	%d1/%d7/%a1-%a6
	rts


PUTSTRING:
	cmpi.l	#0, %d1
	beq	PUTSTRING_INIT
	rts

PUTSTRING_INIT:
	movem.l	%d1/%d7/%a1-%a6, -(%sp)
	move.l	#0, sz
	move.l	sz, %d7
	move.l	%d2, i
	cmp	#0, %d3
	beq	PUTSTRING_LOOP
	
PUTSTRING_LOOP:
	cmp	%d3, %d7
	beq	PUTSTRING_UNMASK

	moveq.l	#1, %d0
	move.l	i, %a6
	move.b	(%a6), %d1
	jsr	INPUT_QUEUE
	cmp	#0, %d0
	beq	PUTSTRING_UNMASK
	
	addq.l	#1, %d7
	addq.l	#1, %a6
	move.l	%d7, sz
	move.l	%a6, i
	bra	PUTSTRING_LOOP

PUTSTRING_UNMASK:
	ori.w	#0xe107, USTCNT1
	bra	PUTSTRING_RETURN

PUTSTRING_RETURN:
	move.l	sz, %d0
	movem.l	(%sp)+,	%d1/%d7/%a1-%a6
	rts


INTERGET:
	movem.l	%d0-%d1, -(%sp)
	cmp	#0, %d1
	beq	INTERGET_INQ
	bra	interget_finish

INTERGET_INQ:
	move.l	#0, %d0
	jsr	INPUT_QUEUE
interget_finish:	
	movem.l	(%sp)+, %d0-%d1
	rts


.section .data
.even
	.equ	SIZE_of_QUEUE,	256
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
sz:	.ds.l	1
i:	.ds.l	1

	.section .bss
WORK:	.ds.b	256


	.end
