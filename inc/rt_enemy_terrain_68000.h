#ifndef RT_ENEMY_TERRAIN_68000_H
#define RT_ENEMY_TERRAIN_68000_H
/* Native 68000 collision-ring lookup. C ABI; only caller-saved registers.
 * Low coordinate words address the ring modulo 64 x 32 cells. */
_Static_assert(sizeof(RT_TerrainMaterial)==8,"material stride");
_Static_assert(__builtin_offsetof(RT_TerrainMaterial,probeCode)==0 && __builtin_offsetof(RT_TerrainMaterial,collision)==1,"material fields");
_Static_assert(__builtin_offsetof(RT_TerrainWindow,materials)==0 && __builtin_offsetof(RT_TerrainWindow,ready)==4100,"terrain ring ABI");
_Static_assert(__builtin_offsetof(RT_EnemyTerrainContext,enemy)==0 && __builtin_offsetof(RT_EnemyTerrainContext,window)==12,"probe context ABI");
_Static_assert(__builtin_offsetof(RT_Enemy,state.x)==0 && __builtin_offsetof(RT_Enemy,state.y)==4 && __builtin_offsetof(RT_Enemy,state.attributes)==14,"enemy ABI");
__asm__(
".text\n"
".even\n"
".globl RT_enemyTerrainProbeReady\n"
"RT_enemyTerrainProbeReady:\n"
"    movea.l 20(%sp),%a0\n"
"    movea.l (%a0),%a1\n"
"    movea.l 12(%a0),%a0\n"
"    move.l %a0,%d0\n"
"    beq .Lterrain_fail\n"
"    tst.b 4100(%a0)\n"
"    beq .Lterrain_fail\n"
"    move.w 2(%a1),%d0\n"
"    lsr.w #7,%d0\n"
"    add.w 6(%sp),%d0\n"
"    andi.w #63,%d0\n"
"    move.w 6(%a1),%d1\n"
"    subq.w #1,%d1\n"
"    lsr.w #7,%d1\n"
"    add.w 10(%sp),%d1\n"
"    andi.w #31,%d1\n"
"    lsl.w #6,%d1\n"
"    add.w %d0,%d1\n"
"    add.w %d1,%d1\n"
"    moveq #0,%d0\n"
"    move.w (%a0,%d1.w),%d0\n"
"    lsl.l #3,%d0\n"
"    lea rtWorldMaterials,%a0\n"
"    adda.l %d0,%a0\n"
"    movea.l 12(%sp),%a1\n"
"    move.b (%a0),(%a1)\n"
"    movea.l 20(%sp),%a1\n"
"    movea.l (%a1),%a1\n"
"    moveq #0,%d1\n"
"    move.b 14(%a1),%d1\n"
"    lsr.w #6,%d1\n"
"    movea.l 16(%sp),%a1\n"
"    move.b 1(%a0,%d1.w),(%a1)\n"
"    moveq #1,%d0\n"
"    rts\n"
".Lterrain_fail:\n"
"    moveq #0,%d0\n"
"    rts\n"
);
#endif
