.global first_task
.global pv_handler
.global P
.global V
.global swtch
.global hard_clock
.global init_timer

** External variables from mtk_c.c **
.extern task_tab
.extern curr_task
.extern next_task
.extern p_body
.extern v_body
.extern ready
.extern sched
.extern addq

.section .text
.even

************************************************************************
** first_task
** Subroutine to start the first task
************************************************************************
first_task:
    move.w  #0x2700, %sr
get_tcm:
    move.l  curr_task, %d0
    mulu.w  #20, %d0
    lea.l   task_tab, %a0
    add.l   %d0, %a0                | %a0 = &task_tab[curr_task]
    move.l  4(%a0), %sp             | restore SSP
    move.l  (%sp)+, %a1             | restore USP from SSP
    move.l  %a1, %usp
    movem.l (%sp)+, %d0-%d7/%a0-%a6 | restore registers from SSP
    rte                             | restore SR and return

************************************************************************
** pv_handler
** trap #1 handler for P and V system calls
************************************************************************
pv_handler:
    move.w  #0x2700, %sr    
    move.l  %d1, -(%sp)         | save the value of argument on top of stack

    cmp.l   #0, %d0
    beq     call_p_body
    cmp.l   #1, %d0
    beq     call_v_body

end_pv_handler:
    move.l  (%sp)+, %d1         
    rte                         | restore SR and return

call_p_body:
    jsr     p_body              | call C's function 'p_body()'
    bra     end_pv_handler
call_v_body:
    jsr     v_body              | call C's function 'v_body()'
    bra     end_pv_handler

************************************************************************
** P
** Entrance of P system call
** Input: semaphore ID
************************************************************************
P:
    movem.l %d0-%d1, -(%sp)
    move.l	#0,	%d0             | d0 <- 0 to call p_body
	move.l	12(%sp), %d1        | get semaphore ID
	trap	#1
    movem.l (%sp)+, %d0-%d1 
	rts
   
************************************************************************
** V
** Entrance of V system call
** Input: semaphore ID
************************************************************************ 
V:
    movem.l %d0-%d1, -(%sp)
    move.l	#1,	%d0             | d0 <- 1 to call v_body
	move.l	12(%sp), %d1        | get semaphore ID
	trap	#1
    movem.l (%sp)+, %d0-%d1
	rts

************************************************************************
** swtch
** Subroutine to switch tasks
************************************************************************
swtch:
    move.w  %sr, -(%sp)
    movem.l %d0-%d7/%a0-%a6, -(%sp)
    move.l  %usp, %a1
    move.l  %a1, -(%sp)

    move.l  curr_task, %d0
    mulu.w  #20, %d0
    lea.l   task_tab, %a0
    add.l   %d0, %a0                | %a0 = &task_tab[curr_task]
    move.l  %sp, 4(%a0)             | save SSP

    move.l  next_task, curr_task    | curr_task = next_task

    bra     get_tcm                 | get the new task's info (similar to first_task)

************************************************************************
** hard_clock
** Subroutine to call at every timer interrupt
************************************************************************
hard_clock:
    movem.l %d0-%d7/%a0-%a6, -(%sp)
    move.l  curr_task, -(%sp)
    move.l  #ready, -(%sp)
    jsr     addq                    | call addq with two arguments on top of stack
    addq.l	#8, %sp                 | pop the curr_task and ready off the stack

    jsr     sched
    jsr     swtch

    movem.l (%sp)+, %d0-%d7/%a0-%a6
    rts

************************************************************************
** init_timer
** Reset and set timer with subroutine call to hard_clock
************************************************************************
init_timer:
	move.l	#3, %d0           | Set to call SYSCALL_NUM_RESET_TIMER
	trap	#0

    move.l	#4, %d0           | Set to call SYSCALL_NUM_SET_TIMER
    move.w	#200, %d1	      | Set cmp value to 10000 (10000*0.01 = 100 ms = 1 second cycle)
	move.l	#hard_clock, %d2    
	trap	#0

    rts
