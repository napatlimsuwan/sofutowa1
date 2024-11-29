
.global inbyte                  | Export inbyte function to other files

.text
.even

inbyte:
    movem.l %d1-%d3, -(%sp)     | Save registers that will be used
inbyte_start:
    move.l	#1, %d0             | System call number for GETSTRING
    move.l	#0, %d1			    | Serial port number = 0
    move.l	#BUF_INBYTE, %d2    | Buffer address
    move.l	#1, %d3             | Buffer size = 1 byte
    trap	#0                  | Initiate system call to GETSTRING

    cmpi.b	#1, %d0             | Check if GETSTRING was successful (1 byte read)
    bne     inbyte_start        | If not, try again

    clr.l	%d0
    move.b  BUF_INBYTE, %d0     | Move byte from buffer to %d0

    movem.l (%sp)+, %d1-%d3     | Restore registers
    rts                         | Return from function

.section .bss
BUF_INBYTE: .ds.b 1             | 1 byte buffer for inbyte function
    .even

/*
.global inbyte                  | Export inbyte function to other files

.text
.even

inbyte:
    movem.l %d1-%d3, -(%sp)     | Save registers that will be used
retry:
    move.l  #1, %d0             | System call number for GETSTRING
    move.l  #0, %d1             | Serial port number = 0
    lea     BUF_INBYTE, %a0     | Load address of buffer into %a0
    move.l  %a0, %d2            | Buffer address
    move.l  #1, %d3             | Buffer size = 1 byte
    trap    #0                  | Initiate system call to GETSTRING

    tst.b   BUF_INBYTE          | Test if a byte was received
    beq     retry               | If not, try again

    clr.l   %d0
    move.b  BUF_INBYTE, %d0     | Move byte from buffer to %d0

    movem.l (%sp)+, %d1-%d3     | Restore registers
    rts                         | Return from function

.section .bss
BUF_INBYTE: .ds.b 1             | 1 byte buffer for inbyte function
.even
*/
