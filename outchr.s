.global outbyte

.text
.even

outbyte:
    move.b  7(%sp), BUF_OUTBYTE
    movem.l %d0-%d3, -(%sp)
outbyte_start:
    move.l	#2, %d0             /* PUTSTRING */
    move.l	#0, %d1
    move.l	#BUF_OUTBYTE, %d2
    move.l	#1, %d3
    trap	#0

    cmpi.b	#1, %d0
    bne     outbyte_start

    movem.l (%sp)+, %d0-%d3
    rts

.section .bss
BUF_OUTBYTE:    .ds.b 1
    .even
