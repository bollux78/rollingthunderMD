#ifndef RT_NATIVE_SURVIVAL_WORLD_H
#define RT_NATIVE_SURVIVAL_WORLD_H
#include "rt_world.h"
#include "rt_survival_stream.h"

/* A native terrain directory for the eight retained Level1 sections. No
 * source CPU state, tile decoding, or collision pixel copies are required.
 * Local x=0 is window.firstChunk; the owner must rebase entities, camera,
 * pending display writes and spawn consumers together before replacing it.
 * This object must remain at a stable address while stage is referenced. */
typedef struct {
    RT_WorldStage stage;
    u16 pages[RT_SURVIVAL_WINDOW_CHUNKS*2*4];
    u8 chunks[RT_SURVIVAL_WINDOW_CHUNKS];
    u32 firstChunk;
    u8 densityTier; /* 0..4; retirement preserves saturation across serial wrap. */
} RT_SurvivalWorld;

/* Builds a complete snapshot, leaving output unchanged on invalid input.
 * Only the original Level1 geometry (608x64 cells) is accepted. Entrances
 * are local placeholders; session placement/respawn belongs to the owner. */
u8 RT_survivalWorldBuild(RT_SurvivalWorld *world,const RT_WorldStage *source,
    const RT_SurvivalWindow *window);
/* Resolve an in-window local tile coordinate for native display/spawn
 * consumers. Does not advance the generator; outputs unchanged on failure. */
u8 RT_survivalWorldSourceCell(const RT_SurvivalWorld *world,s16 localX,
    s16 localY,u16 *sourceX,u16 *sourceY);
#endif
