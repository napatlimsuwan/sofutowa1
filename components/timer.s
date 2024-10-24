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
