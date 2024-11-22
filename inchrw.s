.global inbyte

.text
.even

inbyte:
    movem.l %d1-%d3, -(%sp)
inbyte_start:
    move.l	#1, %d0             /* GETSTRING */
    move.l	#0, %d1			    /*ch = 0*/
    move.l	#BUF_INBYTE, %d2
    move.l	#1, %d3
    trap	#0

    cmpi.b	#1, %d0
    bne     inbyte_start

    clr.l	%d0
    move.b  BUF_INBYTE, %d0
    move.b  %d0, 0x00d00039     /* debug LED0 */

    movem.l (%sp)+, %d1-%d3
    rts

.section .bss
BUF_INBYTE: .ds.b 1
    .even
