********************
** System call numbers 
******************** 
    .equ    SYSCALL_NUM_GETSTRING, 1 
    .equ    SYSCALL_NUM_PUTSTRING, 2 
    .equ    SYSCALL_NUM_RESET_TIMER, 3 
    .equ    SYSCALL_NUM_SET_TIMER, 4 

******************************
** Head of the Register Group
*******************************
    .equ    REGBASE, 0xFFF000 | DMAP is used.
    .equ    IOBASE, 0x00d00000
*******************************
** Registers Related to Interrupts
*******************************
    .equ    IVR, REGBASE+0x300 | Interrupt Vector Register
    .equ    IMR, REGBASE+0x304 | Interrupt Mask Register
    .equ    ISR, REGBASE+0x30c | Interrupt Status Register
    .equ    IPR, REGBASE+0x310 | Interrupt Pending Register
*******************************
** Registers Related to the Timer
*******************************
    .equ    TCTL1, REGBASE+0x600 	|Timer1 Control Register
    .equ    TPRER1, REGBASE+0x602 	|Timer1 Prescaler Register
    .equ    TCMP1, REGBASE+0x604 	|Timer1 Compare Register
    .equ    TCN1, REGBASE+0x608 	|Timer1 Counter Register
    .equ    TSTAT1, REGBASE+0x60a 	|Timer1 Status Register
*******************************
** Registers Related to UART1 (Transmitter and Receiver)
*******************************
    .equ    USTCNT1, REGBASE+0x900 	|UART1 Status / Control Register
    .equ    UBAUD1, REGBASE+0x902 	| UART 1 Baud Control Register
    .equ    URX1, REGBASE+0x904 	| UART 1 Receiver register
    .equ    UTX1, REGBASE+0x906 	| UART 1 Transmitter Register
*******************************
** LED
*******************************
    .equ    LED7, IOBASE+0x000002f 	| Register for LED mounted on the board
    .equ    LED6, IOBASE+0x000002d 	| Refer to Appendix A.4.3.1 for a way to use
    .equ    LED5, IOBASE+0x000002b
    .equ    LED4, IOBASE+0x0000029
    .equ    LED3, IOBASE+0x000003f
    .equ    LED2, IOBASE+0x000003d
    .equ    LED1, IOBASE+0x000003b
    .equ    LED0, IOBASE+0x0000039
    .equ    PUSHSW, 0xFFF419 		| Register for Push Switch mounted on the board
****************************************************************
** Reservation of the stack region
****************************************************************
.section .bss
.even
SYS_STK:
    .ds.b   0x4000  | System stack region
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
    move.w  #0x2700, %SR	    | run at lv.0
    lea.l   SYS_STK_TOP, %SP    | Set SSP
******************************
**Initialization of the interrupt controller
******************************
    move.b  #0x40, IVR                  | Set the user interrupt vector number to 0x40+level.
    move.l  #0x00ff3ff9, IMR            | Allow UART1 and timer interrupts
    move.l  #SYSCALL, 0x080             | Set the interrupt for system call TRAP #0
    move.l  #UART1_INTERRUPT, 0x110     | Set the interrupt subroutine for level 4 interrupt
    move.l  #TIMER_INTERRUPT, 0x118     | Set the interrupt subroutine for level 6 interrupt
******************************
** Initialization related to the transmitter and the receiver (UART1)
** (The interrupt level has been fixed to 4.)
******************************
    move.w  #0x0000, USTCNT1 | Reset
    move.w  #0xe10c, USTCNT1 | Transmission and reception possible - no parity, 1 stop, 8 bit, allow only tranmission interrupt
    move.w  #0x0038, UBAUD1  | baud rate = 230400 bps
*************************
** Initialization related to the timer (The interrupt level has been fixed to 6.)
*************************
    move.w  #0x0004, TCTL1  | Restart, an interrupt impossible
                            | Count the time with the 1/16 of the system clock
                            | as a unit
                            | Stop the timer use
    jsr		INIT
    bra     MAIN
****************************************************************
**    Program region
****************************************************************
MAIN:
    ** Set the running mode and the level (The process to move to 'the user mode')
    move.w	#0x0000, %SR		/*USER MODE, LEVEL 0*/
    lea.l	USR_STK_TOP, %SP	/*set user stack*/
    
    ** Start up RESET_TIMER by the system call
    move.l	#SYSCALL_NUM_RESET_TIMER, %d0
    
    trap	#0
    ** Start up SET_TIMER by the system call
    move.l	#SYSCALL_NUM_SET_TIMER, %d0
    move.w	#50000, %d1
    move.l	#TT, %d2
    trap	#0


************************************* 
*    Test of sys_GETSTRING and sys_PUTSTRING 
*    Echo-back the input from a terminal 
************************************* 

LOOP:
    move.l	#SYSCALL_NUM_GETSTRING, %d0
    move.l	#0, %d1			/*ch = 0*/
    move.l	#BUF, %d2		/*p = #BUF*/
    move.l	#256, %d3		/*size = 256*/
    trap	#0
    move.l	%d0, %d3		/*size = %d0 (The length of a given string)*/
    move.l	#SYSCALL_NUM_PUTSTRING, %d0
    move.l	#0, %d1			/*ch = 0*/
    move.l	#BUF, %d2		/*p = #BUF*/
    trap	#0
    bra		LOOP		

**************************************       
*    Test of the timer       
*    Display ‘******’ and CRLF (Carriage Return, Line Feed) five times       
*    Do RESET_TIMER after five times of the execution       
**************************************   

TT:
    movem.l	%d0-%d7/%a0-%a6, -(%sp)
    cmpi.w	#5, TTC			/*Count with the counter TTC whether five times of the execution have been performed*/
    beq		TTKILL			/*Stop the timer after five times of the execution*/
    move.l	#SYSCALL_NUM_PUTSTRING, %d0
    move.l	#0, %d1			/*ch = 0*/
    move.l	#TMSG, %d2		/*p = #TMSG*/
    move.l	#8, %d3			/*size = 8*/
    trap	#0
    addi.w	#1, TTC			/*Increment TTC counter by 1 and return*/
    bra		TTEND
    
TTKILL:
    move.l	#SYSCALL_NUM_RESET_TIMER, %d0
    trap	#0
    
TTEND:
    movem.l	(%sp)+, %d0-%d7/%a0-%a6
    rts

****************************************************************
**  System Call Interface:
**	Maker: Sihanern Thitisan
**  Reviewer: Loa Champ, Nimrawee Nattapat
****************************************************************
        
SYSCALL:
    cmpi.l  #SYSCALL_NUM_GETSTRING,%d0      | if %d0 == 1
    beq     JUMP_GETSTRING                  | Jump to the subroutine for GETSTRING
    cmpi.l  #SYSCALL_NUM_PUTSTRING,%d0      | if %d0 == 2
    beq     JUMP_PUTSTRING                  | Jump to the subroutine for PUTSTRING
    cmpi.l  #SYSCALL_NUM_RESET_TIMER,%d0    | if %d0 == 3
    beq     JUMP_RESET_TIMER                | Jump to the subroutine for RESET_TIMER
    cmpi.l  #SYSCALL_NUM_SET_TIMER,%d0      | if %d0 == 4
    beq     JUMP_SET_TIMER                  | Jump to the subroutine for SET_TIMER
    rte

JUMP_GETSTRING:
    jsr    GETSTRING                        
    rte
JUMP_PUTSTRING:
    jsr    PUTSTRING
    rte
JUMP_RESET_TIMER:
    jsr    RESET_TIMER
    rte
JUMP_SET_TIMER:
    jsr    SET_TIMER
    rte


****************************************************************
**	Timer interrupt
**	Maker: Nimrawee Nattapat, Loa Champ
**  Reviewer: Sihanern Thitisan, Nam Non
****************************************************************
TIMER_INTERRUPT:                    
    movem.l %a0, -(%sp)         | Save the registers            
    cmp.w   #1,	TSTAT1          | Check 0th bit of TSTAT1 to see if the cycle count has reached compare value
    beq	    TSTAT1_reset        | If last bit = 1 and timeer interrupt is occuring, jump to TSTAT1_reset 
    jmp	    Go_back

RESET_TIMER:
    move.w  #0x0004,    TCTL1   | Restart, an interrupt impossible |Count the time with the 1/16 of the system clock |as a unit |Stop the timer use
    rts

SET_TIMER:
    move.w	#0x0ce,	TPRER1      | Set TPRER1 to a value that allows one cycle to be 0.1 ms
    move.w 	%d1, TCMP1          | Move compare value input in d1 to TCMP1 
    move.w	#0x0015, TCTL1      | Enable timer
    move.l 	%d2, task_p         | move head address of interupt task to a variable called task_p
    rts

Go_back:
    movem.l	(%sp)+, %a0         | Restore the registers
    rte
    
TSTAT1_reset:
    move.w	#0, TSTAT1          | Reset TSTAT1 back to 0 
    jsr		Call_rp             | Jump to Call_rp to perform task_p
    jmp		Go_back

Call_rp:
    move.l 	(task_p), %a0       
    jsr		(%a0)               | jump to the address in task_p
    rts
    
****************************************************************
**	UART1 Interrupt
**	Maker: Sihanern Thitisan, Lee Jiseok
**  Reviewer: Loa Champ, Nimrawee Nattapat
****************************************************************
UART1_INTERRUPT:
    movem.l %d1-%d4, -(%sp)
    move.w	URX1, %d3
    btst.l	#13, %d3                | Check if the 13th bit of URX1 is 1
    bne     RECEIVER_INTERRUPT      | If the 13th bit is 1, it is a receiver interrupt
    move.w  UTX1, %d3
    btst.l  #15, %d3                | Check if the 15th bit of UTX1 is 1
    bne     TRANSMITTER_INTERRUPT
    bra     UART1_INTERRUPT_END

TRANSMITTER_INTERRUPT:
    move.l  #0, %d1                 | Move 0 to %d1
    jsr     INTERPUT                | Jump to INTERPUT subroutine
    bra     UART1_INTERRUPT_END
RECEIVER_INTERRUPT:
    move.l  #0, %d1
    move.b  %d3, %d2                | Prepare arguments for INTERGET
    jsr     INTERGET                | Jump to INTERGET subroutine
UART1_INTERRUPT_END:
    movem.l (%sp)+, %d1-%d4
    rte

****************************************************************
**	INTERGET
**	Maker: Liu Yiluo, Nam Non
**  Reviewer: Lee Jiseok
****************************************************************	
INTERGET:
    cmpi.l  #0, %d1                     | compare the ch, it should be 0
    beq     INTERGET_INQ
    rts

INTERGET_INQ:
    movem.l	%d0-%d2, -(%sp)
    move.l	#0, %d0                     | 0 means the first queue, the reception queue
    move.b	%d2, %d1
    jsr		INPUT_QUEUE                 | jump to the INQ
    movem.l	(%sp)+, %d0-%d2
    rts
        
****************************************************************
**  INTERPUT
**	Maker: Lee Jiseok
**  Reviewer: Liu Yiluo
****************************************************************	
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
        
****************************************************************
**  PUTSTRING
**	Maker: Liu Yiluo, Champ Loa, Nimrawee Nattapat
**  Reviewer: Lee Jiseok
****************************************************************
PUTSTRING:
    cmpi.l	#0, %d1         /*ifch≠0,return without doing anything.*/
    beq	PUTSTRING_INIT
    rts

PUTSTRING_INIT:
    movem.l	%d1/%d7/%a1-%a6, -(%sp)
    /*sz->0,i->p*/
    move.l	#0, size_put        | init
    move.l	size_put, %d7
    move.l	%d2, ptr_put        | head address p
    cmp		#0, %d3
    beq		PUTSTRING_LOOP
    
PUTSTRING_LOOP:     
    cmp		%d3, %d7            | sz = size?
    beq		PUTSTRING_UNMASK

    moveq.l	#1, %d0         /*Execute INQ(1,p[i]) and write in data at the address i into the transmitting queue*/
    move.l	ptr_put, %a6
    move.b	(%a6), %d1
    jsr		INPUT_QUEUE
    cmp		#0, %d0         /*if it failed or full go to unmask*/
    beq		PUTSTRING_UNMASK
    
    /* sz++,i++ */
    addq.l	#1, %d7             | update
    addq.l	#1, %a6             | update
    move.l	%d7, size_put
    move.l	%a6, ptr_put
    bra		PUTSTRING_LOOP

PUTSTRING_UNMASK:
    ori.w	#0xe107, USTCNT1    /*Permit the transmitter interrupt (unmask) manipulating USTCNT1.*/
    bra		PUTSTRING_RETURN

PUTSTRING_RETURN:
    move.l	size_put, %d0       /*%D0 <-sz*/
    movem.l	(%sp)+,	%d1/%d7/%a1-%a6
    rts

****************************************************************
**  GETSTRING
**	Maker: Liu Yiluo
**  Reviewer: Lee Jiseok
****************************************************************
GETSTRING:
    cmpi.l	#0, %d1                             | compare the ch with 0
    beq		GETSTRING_INIT
    rts

GETSTRING_INIT:
    movem.l	%d1/%d7/%a1-%a6, -(%sp)
    move.l	#0, size_get                        | init the sz
    move.l	size_get, %d7
    move.l	%d2, ptr_get
    move.l	ptr_get, %a6                        | store the p at a6, the head address
    bra		GETSTRING_LOOP

GETSTRING_LOOP:
    cmp		%d3, %d7                            | compare whether the sz reaches the size
    beq		GETSTRING_RETURN

    moveq.l	#0, %d0                             | 0 is the first queue
    jsr		OUTPUT_QUEUE
    cmp		#0, %d0                             | to check the output of OUTQ
    beq		GETSTRING_RETURN
    
    move.b	%d1, (%a6)                          | copy the data to the address
    addq.l	#1, %d7
    addq.l	#1, %a6
    move.l	%d7, size_get                       | update
    move.l	%a6, ptr_get                        | update
    bra		GETSTRING_LOOP

GETSTRING_RETURN:
    move.l	size_get, %d0
    movem.l	(%sp)+,	%d1/%d7/%a1-%a6
    rts

*****************************************************************
** Queues
**	Maker: Liu Yiluo, Lee Jiseok
**  Reviewer: Lee Jiseok, Liu Yiluo
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
    move.w 	#0x2700, %SR	/*runlevel->7*/
    
    /*check the no of the que*/
    cmpi.b	#0, %d0
    bne		INPUT_Q1
    bra		INPUT_Q0

INPUT_Q0:				/*routine for the Queue no.0*/
    lea.l	top_0, %a1		/*top address is a1*/
    lea.l	bottom_0, %a2		/*bottom address is a2*/
    move.l 	in_0, %a3		/*pointer in->a3*/
    move.l	out_0, %a4		/*pointer out->a4*/
    move.w	s_0, %d3		/*s->d3*/
    jsr		INQ

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
    jsr		INQ

    /*after the subroutine update the varibles*/
    move.w	%d3, s_1
    move.l	%a3, in_1
    move.w	%d5, %SR
    movem.l	(%sp)+,%d3-%d5/%a1-%a6
    rts

INQ:
    cmp.w	#256, %d3
    bne		INQ_SUCC		/*if s not equals to 256*/
    bra		INQ_FAIL		/*if s==256*/

INQ_SUCC:
    move.b 	%d1, (%a3)

    cmp		%a2, %a3
    beq		INQ_BACK		/*reach the bottom*/
    bra		INQ_NEXT
    
INQ_NEXT:                   | move to the next address
    addq	#1, %a3
    addi.w	#1, %d3
    move.l	#1, %d0
    rts

INQ_BACK:                   | go back to the start of the queue
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
    move.w 	#0x2700, %SR	/*runlevel->7*/
    cmpi.b	#0, %d0			/*check the no of the que*/
    bne		OUTPUT_Q1
    bra		OUTPUT_Q0
    
OUTPUT_Q0:
    lea.l	top_0, %a1		/*top address is a1*/
    lea.l	bottom_0, %a2		/*bottom address is a2*/
    move.l	out_0, %a4		/*pointer out<-a4*/
    move.w	s_0, %d3
    jsr		OUTQ
    
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
    jsr		OUTQ

    /*after the subroutine update the varibles*/
    move.w	%d3, s_1
    move.l	%a4, out_1
    
    move.w	%d5, %SR
    movem.l	(%sp)+,%d3-%d5/%a1-%a6
    rts

OUTQ:
    cmp.w 	#0, %d3
    bne 	OUTQ_SUCC
    bra		OUTQ_FAIL

OUTQ_SUCC:
    move.b	(%a4), %d1

    cmp		%a2, %a4
    beq		OUTQ_BACK		/*reach the bottom*/
    bra		OUTQ_NEXT

OUTQ_NEXT:              | move to the next address
    addq	#1, %a4
    subi.w	#1, %d3	
    move.l	#1, %d0
    rts

OUTQ_BACK:              | back to the start address
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

size_put:	.ds.l	1
ptr_put:	.ds.l	1
size_get:	.ds.l	1
ptr_get:	.ds.l	1

task_p:		.ds.l 1
            .even

****************************************************************
**	Data region with an initial value
****************************************************************
.section .data
TMSG:		.ascii	"******\r\n"
            .even
TTC:		.dc.w	0
            .even

****************************************************************
**	Data region without an initial value
****************************************************************
.section .bss
BUF:		.ds.b	256
            .even
USR_STK:
            .ds.b	0x4000
            .even
USR_STK_TOP:
