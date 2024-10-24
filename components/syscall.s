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
    .equ PUSHSW, 0xFFF419 | Register for Push Switch mounted on the board
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
    move.l #0x00ff3ffb, IMR     | Allow UART1 interrupts
**    move.l #UART1_INTERRUPT, 0x110      |Set the interrupt subroutine for level 4 interrupt
    move.l #SYSCALL, 0x080      | Set the interrupt for system call TRAP #0
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
** Main routine
*****************************************************************
.section .text
.even

MAIN:
    btst.b    #0,    PUSHSW
    beq   CALL_1
    btst.b    #1,    PUSHSW
    beq   CALL_2
    btst.b    #2,    PUSHSW
    beq   CALL_3
    btst.b    #3,    PUSHSW
    beq   CALL_4
    bra MAIN
CALL_1:
    move.l    #1, %d0
    trap   #0
    bra   MAIN
CALL_2:
    move.l    #2, %d0
    trap   #0
    bra   MAIN
CALL_3:
    move.l    #3, %d0
    trap   #0
    bra   MAIN
CALL_4:
    move.l    #4, %d0
    trap   #0
    bra   MAIN


*****************************************************************
** System call interface
*****************************************************************
SYSCALL:
    cmpi.l  #SYSCALL_NUM_GETSTRING,%d0
    beq     JUMP_GETSTRING
    cmpi.l  #SYSCALL_NUM_PUTSTRING,%d0
    beq     JUMP_PUTSTRING
    cmpi.l  #SYSCALL_NUM_RESET_TIMER,%d0
    beq     JUMP_Reset_timer
    cmpi.l  #SYSCALL_NUM_SET_TIMER,%d0
    beq     JUMP_Set_timer
    rte

JUMP_GETSTRING:
    jsr    GETSTRING
    rte
JUMP_PUTSTRING:
    jsr    PUTSTRING
    rte
JUMP_Reset_timer:
    jsr    Reset_timer
    rte
JUMP_Set_timer:
    jsr    Set_timer
    rte

*****************************************************************

GETSTRING:
**    move.b  #'G', LED3
**    move.b  #'E', LED2
**    move.b  #'T', LED1
**    move.b  #'S', LED0
**    rts

PUTSTRING:
**    move.b  #'P', LED3
**    move.b  #'U', LED2
**    move.b  #'T', LED1
**    move.b  #'S', LED0
**    rts

Reset_timer:
**    move.b  #'R', LED3
**    move.b  #'T', LED2
**    move.b  #'M', LED1
**    move.b  #'R', LED0
**    rts

Set_timer:
**    move.b  #'S', LED3
**    move.b  #'T', LED2
**    move.b  #'M', LED1
**    move.b  #'R', LED0
**    rts

.end
