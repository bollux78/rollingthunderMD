#ifndef RT_NATIVE_TILE_CACHE_H
#define RT_NATIVE_TILE_CACHE_H
#include "rt_display.h"
#define RT_TILE_CACHE_PATTERNS 1536
#define RT_TILE_CACHE_SLOTS 768
/* ROM pattern IDs remain separate from physical VDP tile IDs. References are
 * owned by retained name-table cells, including offscreen cells and patches.
 * Zero-reference slots retain useful art in a free list until a miss evicts
 * the oldest unused slot. Lookup and free-slot selection are constant-time. */
typedef struct {
    u16 mapping[RT_TILE_CACHE_PATTERNS]; /* slot+1; zero means absent */
    u16 pattern[RT_TILE_CACHE_SLOTS],references[RT_TILE_CACHE_SLOTS];
    u16 nextFree[RT_TILE_CACHE_SLOTS],previousFree[RT_TILE_CACHE_SLOTS];
    u16 patternCount,slots,firstTile,freeHead,freeTail;
    u8 ready;
} RT_TileCache;
/* Queue one immutable ROM tile before its name-table write. Returning0 must
 * mean no upload was accepted. No source pixels are decoded at runtime. */
typedef u8 (*RT_TileCacheUpload)(u16 pattern,u16 physicalTile,void *context);
u8 RT_tileCacheInitialize(RT_TileCache *cache,u16 patternCount,u16 firstTile,u16 slots);
/* Preserve palette/flip/priority bits, replace only the11-bit pattern field.
 * Acquire once for each retained cell. A failed acquire changes no cache state
 * or output. Release all departing cells before acquiring an entering strip,
 * so the cache needs space for the final window rather than their union. */
u8 RT_tileCacheAcquire(RT_TileCache *cache,u16 attribute,u16 *physicalAttribute,
    RT_TileCacheUpload upload,void *context);
u8 RT_tileCacheRelease(RT_TileCache *cache,u16 physicalAttribute);
/* Frame-skipped queues must keep original write order: tile upload, then
 * copied name-table writes. Slot reuse is legal only at zero final-window
 * references. Publish the complete ordered queue in VBlank, with matching
 * scroll. Never flush half a queued window and expose it as a complete frame. */
#endif
