************************
** INTERGET
    ** d1.l : ch
    ** d2.b : RX data
************************
INTERGET:
    cmpi.l  #0,     %d1
    beq     INTERGET_START
    rts
INTERGET_START:
    movem.l %d1, -(%sp)
    move.l  #0,     %d0
    move.b  %d2,    %d1
    jsr     INPUT_QUEUE
    movem.l (%sp)+, %d1
    rts
