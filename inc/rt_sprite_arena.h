#ifndef RT_NATIVE_SPRITE_ARENA_H
#define RT_NATIVE_SPRITE_ARENA_H
#include "rt_sprites.h"
#define RT_SPRITE_ARENA_BLOCKS 45
/* Sixteen-pattern blocks; no heap allocation. Plan every visible streamed
 * pose together before submitting uploads. Repeated pointers share a slot.
 * Residency is incremental: resident poses keep their blocks and only poses
 * that are not resident are allocated from a FIFO ring, evicting the oldest
 * cached art first. Per-position hints make an unchanged pose an O(1) hit; a
 * persistent pose hash finds residents that moved position. */
typedef struct {
    RT_SpriteSlot slots[RT_SPRITE_ARENA_BLOCKS];
    u16 firstTile;
    u8 blocks,stamp,cursor;
    u8 owner[RT_SPRITE_ARENA_BLOCKS];      /* block -> slot covering it */
    u8 used[RT_SPRITE_ARENA_BLOCKS];       /* slot -> stamp of last planning use */
    u8 chain[RT_SPRITE_ARENA_BLOCKS];      /* resident pose hash chains */
    u8 heads[32];
    u8 hints[80];                          /* plan position -> last slot */
} RT_SpriteArena;
u8 RT_spriteArenaInitialize(RT_SpriteArena *arena,u16 firstTile,u8 blocks);
/* Retains current residency where possible. If fragmentation prevents a set
 * whose total fits, compacts that complete set and invalidates moved poses.
 * Fails (returning 0) only for an invalid pose, more than the block capacity
 * or more unique poses than slots. Uploads publish slot.pose via
 * RT_spriteSubmit; do not queue transfers before planning the whole set. */
u8 RT_spriteArenaPlan(RT_SpriteArena *arena,const RT_SpritePose *const *poses,
    u8 count,u8 *slots);
#endif
