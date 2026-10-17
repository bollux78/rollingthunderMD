#ifndef RT_NATIVE_CAMPAIGN_SCENE_BACKGROUND_H
#define RT_NATIVE_CAMPAIGN_SCENE_BACKGROUND_H
#include <genesis.h>
/* Offline-validated Genesis-native patterns, final tile attributes, palettes.
 * footerCount=0 denotes one static palette, otherwise three raster phases.
 * Maps are two36x28 planes; upload centers them in a64x32 hardware plane.
 * Background patterns occupy [512,512+tileCount); actor banks own other tiles. */
typedef struct {
 const u32 *patterns;
 const u16 (*maps)[1008];
 const u16 (*palettes)[64];
 u16 tileCount,footerFirst,footerCount;
} RT_SceneBackground;
extern const RT_SceneBackground rtSceneBackgrounds[6];
#ifdef __m68k__
/* Exclusive VDP ownership. Caller stops both raster owners and relinquishes
 * queued transfers before entry. Rejects queued DMA / SDK sprite engine.
 * Leaves display blank, with top palette loaded; caller installs scene IRQ,
 * uploads actors and publishes its first SAT before enabling the display.
 * A disabled VDP always asserts its blank status bit: enable at a real VInt,
 * and begin palette IRQ servicing there, not during blanked initialization. */
u8 RT_sceneBackgroundUpload(const RT_SceneBackground *background);
/* Paired callbacks bracket each bounded transfer; caller owns bus timing. */
u8 RT_sceneBackgroundUploadIO(const RT_SceneBackground *background,void (*beginIO)(void),void (*endIO)(void));
#endif
#endif
