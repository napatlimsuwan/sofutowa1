/*
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
*/

.global inbyte                      | Export inbyte function to other files

.text
.even

inbyte:
    movem.l %d1-%d3, -(%sp)         | Save registers %d1, %d2, %d3
inbyte_loop:
    move.l	#1, %d0             | System call number for GETSTRING
    move.l	#0, %d1             | Serial port number = 0
    lea	    	inbyte_buffer, %a0  | Load buffer address into %a0
    move.l	%a0, %d2            | Pass buffer address to %d2
    move.l	#1, %d3             | Buffer size = 1 byte
    trap	#0                  | Initiate system call to GETSTRING
    tst.b   %d0                     | Check if 1 byte was successfully read
    beq     inbyte_loop             | Retry if no byte was read (result is zero)
    move.b  inbyte_buffer, %d0      | Load byte from buffer into %d0
    movem.l (%sp)+, %d1-%d3         | Restore saved registers
    rts                             | Return from function

.section .bss
inbyte_buffer: .ds.b 1              | 1 byte buffer for inbyte function
.even

