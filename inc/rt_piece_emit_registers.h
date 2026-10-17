#ifndef RT_PIECE_EMIT_REGISTERS_H
#define RT_PIECE_EMIT_REGISTERS_H
#include "rt_sprites.h"
/* Register-call emitters for RT_SpritePiece lists. They reproduce the C clip
 * rules and device arithmetic of RT_spriteSubmit / RT_spriteSubmitStage
 * exactly (all word arithmetic, same signed/unsigned comparisons).
 * In:  d0 count, d1 pieces, d2 (u16)localX, d3 (u16)localY,
 *      d5 instance priority (low byte), a0 commands, a1 pieces, a2 order keys,
 *      a3 facing*2, a4 first tile, a5 order key.
 * Each record links to its predecessor's HUD SAT index (count+3). Order
 * keys are implied by the caller's run list; a2/a5 are not used.
 * Out: d0 new count. Clobbers d1,d4,d6,d7,a0-a2. Include from one file only. */
_Static_assert(sizeof(RT_SpritePiece)==22,"piece ABI");
__asm__(
"    .text\n"
"    .align 2\n"
"    .global RT_emitPiecesRegisters\n"
"RT_emitPiecesRegisters:\n"
"    addq.w #2,%d0\n"
"    subq.w #1,%d1\n"
"    bmi .Gdone\n"
".Gpiece:\n"
"    move.b 20(%a1),%d6\n"
"    beq .Grule0\n"
"    cmpi.b #3,%d6\n"
"    beq .Grule3\n"
"    cmpi.b #2,%d6\n"
"    bne .Gplace\n"
/* rule2 pre: -32<=localX<320 and 0<localY<=256 */
"    move.w %d2,%d6\n"
"    addi.w #32,%d6\n"
"    cmpi.w #352,%d6\n"
"    bcc .Gnext\n"
"    move.w %d3,%d6\n"
"    subq.w #1,%d6\n"
"    cmpi.w #256,%d6\n"
"    bcc .Gnext\n"
"    bra .Gplace\n"
".Grule0:\n"
"    move.w 6(%a1,%a3.w),%d6\n"
"    add.w %d2,%d6\n"
"    addi.w #32,%d6\n"
"    cmpi.w #352,%d6\n"
"    bcc .Gnext\n"
"    move.w 10(%a1),%d6\n"
"    sub.w %d3,%d6\n"
"    addi.w #288,%d6\n"
"    cmpi.w #256,%d6\n"
"    bcc .Gnext\n"
"    bra .Gplace\n"
".Grule3:\n"
"    move.w 6(%a1),%d6\n"
"    add.w %d2,%d6\n"
"    addi.w #31,%d6\n"
"    cmpi.w #319,%d6\n"
"    bcc .Gnext\n"
"    move.w 10(%a1),%d6\n"
"    sub.w %d3,%d6\n"
"    addi.w #287,%d6\n"
"    cmpi.w #255,%d6\n"
"    bcc .Gnext\n"
".Gplace:\n"
/* d6=x=(localX+16+px)&511, d7=y=((localY+py-16)&255)-16 */
"    move.w 0(%a1,%a3.w),%d6\n"
"    add.w %d2,%d6\n"
"    addi.w #16,%d6\n"
"    andi.w #511,%d6\n"
"    move.w 4(%a1),%d7\n"
"    add.w %d3,%d7\n"
"    subi.w #16,%d7\n"
"    andi.w #255,%d7\n"
"    subi.w #16,%d7\n"
"    cmpi.b #1,20(%a1)\n"
"    beq .Grule1\n"
"    cmpi.b #2,20(%a1)\n"
"    bne .Gwrite\n"
/* rule2 post: x<304, x+clipX0>16, y<224, y+clipY>0 */
"    cmpi.w #304,%d6\n"
"    bcc .Gnext\n"
"    move.w 6(%a1),%d4\n"
"    add.w %d6,%d4\n"
"    cmpi.w #16,%d4\n"
"    ble .Gnext\n"
"    cmpi.w #224,%d7\n"
"    bge .Gnext\n"
"    move.w 10(%a1),%d4\n"
"    add.w %d7,%d4\n"
"    ble .Gnext\n"
"    bra .Gwrite\n"
".Grule1:\n"
"    cmpi.w #304,%d6\n"
"    bcc .Gnext\n"
"    moveq #12,%d4\n"
"    and.b 18(%a1),%d4\n"
"    add.w %d4,%d4\n"
"    addq.w #8,%d4\n"
"    add.w %d6,%d4\n"
"    cmpi.w #16,%d4\n"
"    ble .Gnext\n"
"    cmpi.w #224,%d7\n"
"    bge .Gnext\n"
"    moveq #3,%d4\n"
"    and.b 18(%a1),%d4\n"
"    lsl.w #3,%d4\n"
"    addq.w #8,%d4\n"
"    add.w %d7,%d4\n"
"    ble .Gnext\n"
".Gwrite:\n"
"    addi.w #128,%d7\n"
"    move.w %d7,(%a0)+\n"
"    move.b 18(%a1),(%a0)+\n"
"    addq.w #1,%d0\n"
"    move.b %d0,(%a0)+\n"
"    move.w 14(%a1,%a3.w),%d7\n"
"    add.w %a4,%d7\n"
"    add.w 12(%a1),%d7\n"
"    move.b 19(%a1),%d4\n"
"    or.b %d5,%d4\n"
"    cmpi.b #3,%d4\n"
"    bcs .Gattr\n"
"    bset #15,%d7\n"
".Gattr:\n"
"    move.w %d7,(%a0)+\n"
"    addi.w #128,%d6\n"
"    move.w %d6,(%a0)+\n"

".Gnext:\n"
"    lea 22(%a1),%a1\n"
"    dbra %d1,.Gpiece\n"
".Gdone:\n"
"    subq.w #2,%d0\n"
"    rts\n"
/* Stage records: rule3 for every piece, facing zero (RT_spriteSubmitStage). */
"    .global RT_emitStageRegisters\n"
"RT_emitStageRegisters:\n"
"    addq.w #2,%d0\n"
"    subq.w #1,%d1\n"
"    bmi .Sdone\n"
".Spiece:\n"
"    move.w 6(%a1),%d6\n"
"    add.w %d2,%d6\n"
"    addi.w #31,%d6\n"
"    cmpi.w #319,%d6\n"
"    bcc .Snext\n"
"    move.w 10(%a1),%d6\n"
"    sub.w %d3,%d6\n"
"    addi.w #287,%d6\n"
"    cmpi.w #255,%d6\n"
"    bcc .Snext\n"
"    move.w 4(%a1),%d7\n"
"    add.w %d3,%d7\n"
"    subi.w #16,%d7\n"
"    andi.w #255,%d7\n"
"    addi.w #112,%d7\n"
"    move.w %d7,(%a0)+\n"
"    move.b 18(%a1),(%a0)+\n"
"    addq.w #1,%d0\n"
"    move.b %d0,(%a0)+\n"
"    move.w 14(%a1),%d7\n"
"    add.w %a4,%d7\n"
"    add.w 12(%a1),%d7\n"
"    move.b 19(%a1),%d4\n"
"    or.b %d5,%d4\n"
"    cmpi.b #3,%d4\n"
"    bcs .Sattr\n"
"    bset #15,%d7\n"
".Sattr:\n"
"    move.w %d7,(%a0)+\n"
"    move.w (%a1),%d6\n"
"    add.w %d2,%d6\n"
"    addi.w #16,%d6\n"
"    andi.w #511,%d6\n"
"    addi.w #128,%d6\n"
"    move.w %d6,(%a0)+\n"

".Snext:\n"
"    lea 22(%a1),%a1\n"
"    dbra %d1,.Spiece\n"
".Sdone:\n"
"    subq.w #2,%d0\n"
"    rts\n"
);
static inline u16 RT_emitPiecesCall(u8 stage,RT_SpriteCommand *command,u8 *order,
    const RT_SpritePiece *piece,u16 count,u16 n,u16 x,u16 y,u16 base,u8 priority,u8 key,u8 facing)
{
    register u32 d0 __asm__("d0")=count;
    register u32 d1 __asm__("d1")=n;
    register u32 d2 __asm__("d2")=x;
    register u32 d3 __asm__("d3")=y;
    register u32 d5 __asm__("d5")=priority;
    register RT_SpriteCommand *a0 __asm__("a0")=command;
    register const RT_SpritePiece *a1 __asm__("a1")=piece;
    register u8 *a2 __asm__("a2")=order;
    register u32 a3 __asm__("a3")=(u32)facing*2;
    register u32 a4 __asm__("a4")=base;
    register u32 a5 __asm__("a5")=key;
    if(stage)
        __asm__ volatile("jsr RT_emitStageRegisters" :
            "+d"(d0),"+d"(d1),"+a"(a0),"+a"(a1),"+a"(a2) :
            "d"(d2),"d"(d3),"d"(d5),"a"(a3),"a"(a4),"a"(a5) : "d4","d6","d7","cc","memory");
    else
        __asm__ volatile("jsr RT_emitPiecesRegisters" :
            "+d"(d0),"+d"(d1),"+a"(a0),"+a"(a1),"+a"(a2) :
            "d"(d2),"d"(d3),"d"(d5),"a"(a3),"a"(a4),"a"(a5) : "d4","d6","d7","cc","memory");
    return d0;
}
#endif
