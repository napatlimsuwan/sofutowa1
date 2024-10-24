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
    .ds.b 0x4000    | System stack region
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
    move.w #0x2000, %SR		| run at lv.0
    lea.l SYS_STK_TOP, %SP  |Set SSP
******************************
**Initialization of the interrupt controller
******************************
    move.b #0x40, IVR           | Set the user interrupt vector
                                | number to 0x40+level.
    move.l #0x00ff3ffb, IMR    | Allow UART1 interrupts
    move.l #UART1_INTERRUPT, 0x110      |Set the interrupt subroutine for level 4 interrupt
******************************
** Initialization related to the transmitter and the receiver (UART1)
** (The interrupt level has been fixed to 4.)
******************************
    move.w #0x0000, USTCNT1 | Reset
    move.w #0xe100, USTCNT1 | Transmission and reception possible
                            | no parity, 1 stop, 8 bit
                            | allow only tranmission interrupt
    move.w #0x0038, UBAUD1  | baud rate = 230400 bps
*************************
** Initialization related to the timer (The interrupt level has been fixed to 6.)
*************************
    move.w #0x0004, TCTL1               | Restart, an interrupt impossible
                                        |Count the time with the 1/16 of the system clock
                                        |as a unit
                                        |Stop the timer use
    bra MAIN
*****************************************************************
* Write in ‘a’ in the transmitter register UTX1 to confirm the normal initialization routine
* operation at the present step. When ‘a’ is outputted, it’s OK.
*****************************************************************
.section .text
.even
MAIN :
    jsr 	INIT

	move.b  #0, %d0
	move.b  #0x61, %d1
	jsr 	INPUT_QUEUE
    move.b  #0x60, %d1
    move.w  #256, %d3
CHANGE_CHAR:
    move.b  #16, %d2
    addq.b  #1, %d1
PUSH_QUEUE:
    subq.b  #1, %d2
    subq.w  #1, %d3
    jsr     INPUT_QUEUE
    cmpi.w  #0, %d3
    beq     CHECK_Q_SIZE
    cmpi.b  #0, %d2
    beq     CHANGE_CHAR
    bra     PUSH_QUEUE

CHECK_Q_SIZE:
    ** check if s_1 is 256
	
	move.b	s_1, LED7
	cmpi.w  #SIZE_of_QUEUE, s_1
	bne     LOOP
	move.b  #'f', LED7
    move.b  #'u', LED6
    move.b  #'l', LED5
    move.b  #'l', LED4
	move.w 	#0xe13f, USTCNT1
    cmpi.w  #0x2700, %d6
    bne     LOOP
    move.b  #'2', LED3
    move.b  #'7', LED2
    move.b  #'0', LED1
    move.b  #'0', LED0

LOOP :
    bra LOOP    
    
*****************************************************************
** UART1 Interrupt
*****************************************************************
UART1_INTERRUPT:
    movem.l %d1, -(%sp)
    move.w  UTX1, %d1
    btst.b  #15, %d1
    beq     UART1_INTERRUPT_SKIP
    move.b  #' ', LED3
    move.b  #'p', LED2
    move.b  #'u', LED1
    move.b  #'t', LED0
    move.l  #0, %d1
    jsr     INTERPUT
UART1_INTERRUPT_END:
**  move.b  #'E', LED7
**  move.b  #'n', LED6
**  move.b  #'d', LED5
    movem.l (%sp)+, %d1
    rte
UART1_INTERRUPT_SKIP:
**	move.b  #'S', LED7
    move.b  #'k', LED6
    move.b  #'i', LED5
	move.b  #'p', LED4
    movem.l (%sp)+, %d1
    rte

INTERPUT:
	move.w #0x2700, %SR	/*runlevel->7*/

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
	movem.l	(%sp)+, %d0-%d1/%a1
	rts

inter_put2:
**	move.l 	UTX1, %d2
**	cmpi	#0, s_1
**	bne	not_empty
	move.w	#0x0800, %d2
**	bra	inter_put3
**not_empty:	
**	add.w	#0x8800, %d2

inter_put3:
	move.b	%d1, %d2	/*d2<-d1+d2*/
	move.w	%d2, UTX1
	movem.l	(%sp)+, %d0-%d1/%a1
	rts

INIT:
	movem.l	%a1, -(%sp)
	lea.l	top_0, %a1		/*top address is a1*/
	move.l	%a1, in_0
	move.l	%a1, out_0
	move.w	#0, s_0

	lea.l	top_1, %a1		/*top address is a1*/
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

.end
