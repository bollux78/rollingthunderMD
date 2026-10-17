#ifndef RT_NATIVE_CAMPAIGN_TRANSITION_ART_H
#define RT_NATIVE_CAMPAIGN_TRANSITION_ART_H
#include "rt_campaign_transition_scene.h"
#include "rt_sprites.h"
/* Offline-decoded transition atlas. No source descriptors or color lookup
 * tables are decoded by the native renderer. Coordinates retain the original
 * component acceptance test before device wrapping and viewport clipping. */
typedef struct {s16 dx,dy;u16 attribute;u8 sourceHeight,left,top,size;} RT_TransitionPart;
typedef struct {u16 first,count;} RT_TransitionPose;
typedef struct {
    const RT_TransitionPart *parts;
    const u32 *patterns;
    const u16 *palette;
    RT_TransitionPose poses[15]; /* Native kinds17..31. */
    u16 partCount,tileCount,upperTile;
} RT_TransitionArt;
extern const RT_TransitionArt rtTransitionArt[6];
/* Prepares the high-actor SAT prefix; low actors must be appended and checked
 * against combined H40 limits before publishing. No VDP writes or clock ticks.
 * Caller-owned scratch avoids a900-byte stack allocation. On failure, batch
 * is marked failed and cannot be submitted. Patterns/background must already
 * be resident: first256 patterns at tile256, remainder at art.upperTile. */
u8 RT_transitionSpritesPrepare(const RT_TransitionArt *art,const RT_StagePool *pool,
    RT_SpriteBatch *batch,u32 budgetDelta[225]);
#ifdef __m68k__
/* Exclusive scene owner, display blank: synchronous pattern/palette upload.
 * Background data and split-palette IRQ installation belong to the scene. */
u8 RT_transitionArtUpload(const RT_TransitionArt *art);
/* Paired callbacks bracket each bounded transfer; caller owns bus timing. */
u8 RT_transitionArtUploadIO(const RT_TransitionArt *art,void (*beginIO)(void),void (*endIO)(void));
/* VBlank or disabled display only. Consumes prepared batch once. */
u8 RT_transitionSpritesPublish(RT_SpriteBatch *batch);
#endif
#endif
