#ifndef RT_ENEMY_EMIT_REGISTERS_H
#define RT_ENEMY_EMIT_REGISTERS_H
#include "rt_sprites.h"
#include "rt_enemies.h"
/* Held enemy group: same layout as campaign_sprite_reprojection.c's HeldGroup. */
typedef struct {
    const RT_SpritePose *pose;
    const RT_PackedSpritePiece *packed;
    u16 tile;
    u8 type,slot,kind,tag;
} RT_HeldGroup;
_Static_assert(sizeof(RT_HeldGroup)==14,"held group ABI");
_Static_assert(__builtin_offsetof(RT_Enemy,state.kind)==8 && __builtin_offsetof(RT_Enemy,state.action)==9 &&
    __builtin_offsetof(RT_Enemy,state.attributes)==14 && __builtin_offsetof(RT_Enemy,state.hidden)==16 &&
    __builtin_offsetof(RT_Enemy,byte4)==30 && sizeof(RT_Enemy)==36 && RT_ENEMY_FREE==255,"enemy ABI");
_Static_assert(__builtin_offsetof(RT_SpritePose,pieceCount)==10,"pose ABI");
_Static_assert(__builtin_offsetof(RT_SpriteBatch,commands)==640 && __builtin_offsetof(RT_SpriteBatch,count)==644 &&
    __builtin_offsetof(RT_SpriteBatch,reserved)==649 && __builtin_offsetof(RT_SpriteBatch,runCount)==730 &&
    __builtin_offsetof(RT_SpriteBatch,runStarts)==731 && __builtin_offsetof(RT_SpriteBatch,runKeys)==811,"batch ABI");
/* Camera for the current call (word, low 16 bits are what matter). */
extern u16 rtEnemyEmitCameraX,rtEnemyEmitCameraY;
/* Emit every held packed enemy in [a2,a3): exact equivalent of the C
 * reprojection loop (identity check, hidden/free skip, room check, packed
 * clip/emit, run marking). a4 enemy slots, a5 batch. d0 = 1 ok / 0 reject. */
__asm__(
"    .text\n"
"    .align 2\n"
"    .global RT_emitHeldEnemies\n"
"RT_emitHeldEnemies:\n"
"    movem.l %d2-%d7/%a2-%a5,-(%sp)\n"
"    move.w 644(%a5),%d0\n"
".Hgroup:\n"
"    cmpa.l %a3,%a2\n"
"    bcc .Hok\n"
"    moveq #0,%d6\n"
"    move.b 11(%a2),%d6\n"
"    mulu.w #36,%d6\n"
"    lea 0(%a4,%d6.l),%a0\n"
"    move.b 8(%a0),%d6\n"
"    cmpi.b #-1,%d6\n"
"    beq .Hnext\n"
"    cmp.b 12(%a2),%d6\n"
"    bne .Hfail\n"
"    move.b 30(%a0),%d6\n"
"    cmp.b 13(%a2),%d6\n"
"    bne .Hfail\n"
"    tst.b 16(%a0)\n"
"    bne .Hnext\n"
"    movea.l (%a2),%a1\n"
"    moveq #0,%d1\n"
"    move.b 10(%a1),%d1\n"
"    moveq #80,%d6\n"
"    moveq #0,%d7\n"
"    move.b 649(%a5),%d7\n"
"    sub.w %d7,%d6\n"
"    sub.w %d0,%d6\n"
"    cmp.w %d1,%d6\n"
"    blt .Hfail\n"
"    move.l (%a0),%d2\n"
"    asr.l #4,%d2\n"
"    sub.w rtEnemyEmitCameraX,%d2\n"
"    moveq #15,%d3\n"
"    add.l 4(%a0),%d3\n"
"    asr.l #4,%d3\n"
"    sub.w rtEnemyEmitCameraY,%d3\n"
"    move.w 8(%a2),%d4\n"
"    move.b 14(%a0),%d5\n"
"    lsr.b #5,%d5\n"
"    movea.l 4(%a2),%a1\n"
"    btst #1,9(%a0)\n"
"    beq.s .Hfacing\n"
"    move.w %d1,%d6\n"
"    mulu.w #12,%d6\n"
"    adda.l %d6,%a1\n"
".Hfacing:\n"
"    movea.l 640(%a5),%a0\n"
"    move.w %d0,%d6\n"
"    lsl.w #3,%d6\n"
"    adda.w %d6,%a0\n"
"    move.w %d0,-(%sp)\n"
"    addq.w #2,%d0\n"
"    subq.w #1,%d1\n"
"    bmi .Hdone\n"
".Hpiece:\n"
"    move.w (%a1),%d6\n"
"    add.w %d2,%d6\n"
"    cmpi.w #352,%d6\n"
"    bcc.s .Hskip\n"
"    move.w 2(%a1),%d6\n"
"    sub.w %d3,%d6\n"
"    cmpi.w #256,%d6\n"
"    bcc.s .Hskip\n"
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
"    bcs.s .Hattr\n"
"    bset #15,%d7\n"
".Hattr:\n"
"    move.w %d7,(%a0)+\n"
"    move.w 4(%a1),%d6\n"
"    add.w %d2,%d6\n"
"    andi.w #511,%d6\n"
"    addi.w #128,%d6\n"
"    move.w %d6,(%a0)+\n"
".Hskip:\n"
"    lea 12(%a1),%a1\n"
"    dbra %d1,.Hpiece\n"
".Hdone:\n"
"    subq.w #2,%d0\n"
"    move.w (%sp)+,%d7\n"
"    cmp.w %d7,%d0\n"
"    beq.s .Hstore\n"
"    move.b %d5,%d6\n"
"    lsl.b #2,%d6\n"
"    addq.b #1,%d6\n"
"    moveq #0,%d1\n"
"    move.b 730(%a5),%d1\n"
"    beq.s .Hnewrun\n"
"    lea 810(%a5),%a1\n"
"    cmp.b 0(%a1,%d1.w),%d6\n"
"    beq.s .Hstore\n"
".Hnewrun:\n"
"    lea 731(%a5),%a1\n"
"    move.b %d7,0(%a1,%d1.w)\n"
"    lea 811(%a5),%a1\n"
"    move.b %d6,0(%a1,%d1.w)\n"
"    addq.b #1,730(%a5)\n"
".Hstore:\n"
"    move.w %d0,644(%a5)\n"
".Hnext:\n"
"    lea 14(%a2),%a2\n"
"    bra .Hgroup\n"
".Hok:\n"
"    moveq #1,%d0\n"
"    movem.l (%sp)+,%d2-%d7/%a2-%a5\n"
"    rts\n"
".Hfail:\n"
"    moveq #0,%d0\n"
"    movem.l (%sp)+,%d2-%d7/%a2-%a5\n"
"    rts\n"
);
static inline u8 RT_emitHeldEnemies(RT_SpriteBatch *batch,const RT_Enemy *slots,
    const RT_HeldGroup *first,const RT_HeldGroup *end)
{
    register u32 d0 __asm__("d0");
    register const RT_HeldGroup *a2 __asm__("a2")=first;
    register const RT_HeldGroup *a3 __asm__("a3")=end;
    register const RT_Enemy *a4 __asm__("a4")=slots;
    register RT_SpriteBatch *a5 __asm__("a5")=batch;
    __asm__ volatile("jsr RT_emitHeldEnemies" : "=d"(d0) :
        "a"(a2),"a"(a3),"a"(a4),"a"(a5) : "d1","a0","a1","cc","memory");
    return d0;
}
#endif
