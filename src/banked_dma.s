/* Flush queued raw-pose DMA without allocating a decode buffer. The caller
 * masks interrupts and guards Z80 bus access. Window 7 is restored after each
 * far transfer; the DMA trigger is always a memory-to-memory write. */
    .text
    .align 2
    .global RT_flushBankedQueue
RT_flushBankedQueue:
    movem.l %d2-%d3/%a2,-(%sp)
    move.l 16(%sp),%a0
    move.l 20(%sp),%d0
    move.l 24(%sp),%d3
    lea 0xc00004,%a1
    lea 0xa130ff,%a2
    subq.w #1,%d0
    bmi.s .Ldone
.Lnext:
    move.l 8(%a0),%d1
    btst #30,%d1
    bne.s .Lfar
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    dbra %d0,.Lnext
    bra.s .Ldone
.Lfar:
    move.l %d1,%d2
    rol.l #8,%d2
    and.b #63,%d2
    move.b %d2,(%a2)
    move.l (%a0)+,(%a1)
    move.l (%a0)+,(%a1)
    and.l #0x00ffffff,%d1
    or.l #0x97000000,%d1
    move.l %d1,(%a1)
    addq.l #4,%a0
    move.l (%a0)+,(%a1)
    move.b %d3,(%a2)
    dbra %d0,.Lnext
.Ldone:
    movem.l (%sp)+,%d2-%d3/%a2
    rts
