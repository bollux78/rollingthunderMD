#ifndef RT_NATIVE_SPRITE_CACHE_H
#define RT_NATIVE_SPRITE_CACHE_H
#include "rt_sprites.h"
#define RT_SPRITE_CACHE_CAPACITY 8
/* Fixed VRAM slots, retained across frames. Plan the complete visible set
 * before submitting any sprite so a later survivor cannot be evicted. */
typedef struct {
    RT_SpriteSlot slots[RT_SPRITE_CACHE_CAPACITY];
    u8 owners[RT_SPRITE_CACHE_CAPACITY];
    u8 count;
} RT_SpriteCache;
u8 RT_spriteCacheInitialize(RT_SpriteCache *cache,u16 firstTile,u8 count,u8 tilesPerSlot);
/* Unique stable actor IDs0..254; outputs slot indices in the same order.
 * Rejects overflow/duplicate IDs without modifying cache or output. Hidden
 * actors need no slot. A complete new visible set may reclaim every old slot. */
u8 RT_spriteCachePlan(RT_SpriteCache *cache,const u8 *actors,u8 count,u8 *slots);
/* Content residency plan: repeated pose pointers share one upload/VRAM slot.
 * Reserve the complete set before any submission; slots[].pose changes only
 * after successful uploads in RT_spriteSubmit. No mutation on plan failure.
 * Use uniform slot capacities; do not mix actor and pose planners in a frame. */
u8 RT_spriteCachePlanPoses(const RT_SpriteCache *cache,
    const RT_SpritePose *const *poses,u8 count,u8 *slots);
#endif
