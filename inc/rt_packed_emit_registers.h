#ifndef RT_PACKED_EMIT_REGISTERS_H
#define RT_PACKED_EMIT_REGISTERS_H
#include "rt_sprites.h"
/* Kept with the register-call wrapper so every native sprites.c build
 * includes the same implementation without an extra linker input. */
__asm__(
"    .text\n"
"    .align 2\n"
"    .global RT_emitPackedRegisters\n"
"RT_emitPackedRegisters:\n"
/* d0 is biased by 2 so each record links to its predecessor's HUD SAT index.
 * Order keys are implied by the caller's run list; a2 is not written. */
"    addq.w #2,%d0\n"
"    subq.w #1,%d1\n"
"    bmi.s .Ldone\n"
".Lpiece:\n"
"    move.w (%a1),%d6\n"
"    add.w %d2,%d6\n"
"    cmpi.w #352,%d6\n"
"    bcc.s .Lnext\n"
"    move.w 2(%a1),%d6\n"
"    sub.w %d3,%d6\n"
"    cmpi.w #256,%d6\n"
"    bcc.s .Lnext\n"
"    move.w 6(%a1),%d6\n"
"    add.w %d3,%d6\n"
"    andi.w #255,%d6\n"
"    addi.w #112,%d6\n"
"    move.w %d6,(%a0)+\n"
"    move.b 10(%a1),(%a0)+\n"
"    addq.w #1,%d0\n"
"    move.b %d0,(%a0)+\n"
"    move.w 8(%a1),%d7\n"
"    add.w %d4,%d7\n"
"    move.b 11(%a1),%d6\n"
"    or.b %d5,%d6\n"
"    cmpi.b #3,%d6\n"
"    bcs.s .Lattribute\n"
"    bset #15,%d7\n"
".Lattribute:\n"
"    move.w %d7,(%a0)+\n"
"    move.w 4(%a1),%d6\n"
"    add.w %d2,%d6\n"
"    andi.w #511,%d6\n"
"    addi.w #128,%d6\n"
"    move.w %d6,(%a0)+\n"
".Lnext:\n"
"    lea 12(%a1),%a1\n"
"    dbra %d1,.Lpiece\n"
".Ldone:\n"
"    subq.w #2,%d0\n"
"    rts\n"
);
static inline u16 RT_emitPackedFast(RT_SpriteCommand *command,u8 *order,const RT_PackedSpritePiece *piece,u16 count,u16 n,u16 x,u16 y,u16 base,u8 priority,u8 key)
{
    register u32 d0 __asm__("d0")=count;
    register u32 d1 __asm__("d1")=n;
    register u32 d2 __asm__("d2")=x;
    register u32 d3 __asm__("d3")=y;
    register u32 d4 __asm__("d4")=base;
    register u32 d5 __asm__("d5")=((u16)key<<8)|priority;
    register RT_SpriteCommand *a0 __asm__("a0")=command;
    register const RT_PackedSpritePiece *a1 __asm__("a1")=piece;
    register u8 *a2 __asm__("a2")=order;
    __asm__ volatile("jsr RT_emitPackedRegisters" :
        "+d"(d0),"+d"(d1),"+a"(a0),"+a"(a1),"+a"(a2) :
        "d"(d2),"d"(d3),"d"(d4),"d"(d5) : "d6","d7","cc","memory");
    return d0;
}
#endif
