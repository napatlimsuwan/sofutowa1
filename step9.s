******************** 
** System call numbers 
******************** 
.equ    SYSCALL_NUM_GETSTRING, 1 
.equ    SYSCALL_NUM_PUTSTRING, 2 
.equ    SYSCALL_NUM_RESET_TIMER, 3 
.equ    SYSCALL_NUM_SET_TIMER, 4       
****************************************** 
*** Program region 
****************************************** 
.section    .text 
.even 

MAIN : 
**    Set the running mode and the level (The process to move to ‘the user mode’) 
    move.w    #0x0000,     %SR  |USER MODE, LEVEL 0 
    lea.l    USR_STK_TOP,  %SP  |set user stack 
**    Start up RESET_TIMER by the system call   
    move.l    #SYSCALL_NUM_RESET_TIMER,    %D0 
    trap    #0 

**    Start up SET_TIMER by the system call 
    move.l    #SYSCALL_NUM_SET_TIMER,    %D0 
    move.w    #50000,    %D1 
    move.l    #TT,    %D2 
    trap    #0 
************************************* 
*    Test of sys_GETSTRING and sys_PUTSTRING 
*    Echo-back the input from a terminal 
************************************* 
LOOP : 
    move.l    #SYSCALL_NUM_GETSTRING,    %D0 
    move.l    #0,    %D1    |ch = 0 
    move.l    #BUF,  %D2    |p = #BUF 
    move.l    #256,  %D3    |size = 256 
    trap    #0 

    move.l    %D0, %D3      |size = %D0 (The length of a given string)      
    move.l    #SYSCALL_NUM_PUTSTRING,    %D0 
    move.l    #0,    %D1    |ch = 0       
    move.l    #BUF, %D2     |p = #BUF       
    trap    #0       
    bra    LOOP      
    
**************************************       
*    Test of the timer       
*    Display ‘******’ and CRLF (Carriage Return, Line Feed) five times       
*    Do RESET_TIMER after five times of the execution       
**************************************       
     
TT :       
    movem.l    %D0 - %D7 / %A0 - %A6,   -(%SP)       
    cmpi.w    #5,    TTC        |Count with the counter TTC whether five times of the execution |have been performed.       
    
    beq    TTKILL               |Stop the timer after five times of the execution.       
    
    move.l    #SYSCALL_NUM_PUTSTRING,    %D0       
    move.l    #0,    %D1        |ch = 0       
    move.l    #TMSG, %D2        | p = #TMSG       
    move.l    #8,    %D3        | size = 8 
    trap    #0         
    addi.w    #1,    TTC        |Increment TTC counter by 1       
    bra    TTEND                |and return      
    
TTKILL :       
    move.l    #SYSCALL_NUM_RESET_TIMER,    %D0       
    trap    #0       

TTEND :      
    movem.l    (%SP)+,    %D0 - %D7 / %A0 - %A6       
    rts       
    
    
****************************************       
***    Data region with an initial value       
****************************************       

.section    .data       

TMSG :
    .ascii    "*****\r\n"       |\r: to the line head (carriage return)       
    .even                       |\n: to the next line (line feed)       
    
TTC :       
    .dc.w      0       
    .even 
      
***************************************       
***    Data region without an initial value       
***************************************       

.section    .bss       

BUF :       
    .ds.b    256         |BUF [256]       
    .even           

USR_STK :       
    .ds.b    0x4000      |User stack region       
    .even       

USR_STK_TOP :            |The end of the user stack regi