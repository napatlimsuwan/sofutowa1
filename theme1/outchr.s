
.global outbyte                 | Export inbyte function to other files

.text
.even

outbyte:
    ** First argument (long word size) from C program is offset 4 from the top of the stack **
    ** to get the byte size argument, we need to offset 3 more bytes from the beginning of the first argument **
    ** so 7 is added to the stack pointer to get the byte sized argument sent from the C program **
    move.b  7(%sp), BUF_OUTBYTE | Get outbyte argument from stack
    movem.l %d0-%d3, -(%sp)     | Save registers that will be used
outbyte_start:
    move.l	#2, %d0             | System call number for PUTSTRING
    move.l	#0, %d1             | Serial port number = 0
    move.l	#BUF_OUTBYTE, %d2   | Buffer address
    move.l	#1, %d3             | Buffer size = 1 byte
    trap	#0                  | Initiate system call to PUTSTRING

    cmpi.b	#1, %d0             | Check if PUTSTRING was successful (1 byte read)
    bne     outbyte_start       | If not, try again

    movem.l (%sp)+, %d0-%d3     | Restore registers
    rts                         | Return from function

.section .bss
BUF_OUTBYTE:    .ds.b 1         | 1 byte Buffer for outbyte function
    .even

/*
.global outbyte                 | Export outbyte function to other files

.text
.even

outbyte:
    move.b  7(%sp), %d0          | Get outbyte argument from stack into %d0
    movem.l %d0-%d3, -(%sp)      | Save registers that will be used
retry:
    move.l  #2, %d0              | System call number for PUTSTRING
    move.l  #0, %d1              | Serial port number = 0
    lea     BUF_OUTBYTE, %a0     | Load address of buffer into %a0
    move.b  %d0, (%a0)           | Move byte to buffer
    move.l  %a0, %d2             | Buffer address
    move.l  #1, %d3              | Buffer size = 1 byte
    trap    #0                   | Initiate system call to PUTSTRING

    tst.b   BUF_OUTBYTE          | Test if a byte was successfully written
    beq     retry                | If not, try again

    movem.l (%sp)+, %d0-%d3      | Restore registers
    rts                          | Return from function

.section .bss
BUF_OUTBYTE:    .ds.b 1         | 1 byte Buffer for outbyte function
.even
*/
