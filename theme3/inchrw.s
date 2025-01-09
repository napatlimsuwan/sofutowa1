.global inbyte
.global inkeyw

.text
.even

inbyte:
    movem.l %d1-%d3, -(%sp)     | char inbyte(int ch)
inbyte_start:
    move.l	#1, %d0             | GETSTRING
    move.l	16(%sp), %d1		| get the first arg (ch)	    
    move.l	#BUF_INBYTE, %d2
    move.l	#1, %d3
    trap	#0

    cmpi.b	#1, %d0
    bne     inbyte_start

    clr.l	%d0
    move.b  BUF_INBYTE, %d0
    | no need to clear the buffer in inbyte
    movem.l (%sp)+, %d1-%d3
    rts

inkeyw:
    movem.l %d1-%d3, -(%sp)     | char inkeyw(int ch)

    move.l	#1, %d0             | GETSTRING 
    move.l	16(%sp), %d1		| get the first arg (ch)	    
    move.l	#BUF_INKEY, %d2
    move.l	#1, %d3
    trap	#0

    clr.l	%d0
    move.b  BUF_INKEY, %d0
    move.b  #0, BUF_INKEY       | clear the buffer

    movem.l (%sp)+, %d1-%d3
    rts

.section .bss
BUF_INBYTE: .ds.b 1             | buffer for inbyte
    .even
BUF_INKEY: .ds.b 1              | buffer for inkeyw           
    .even
