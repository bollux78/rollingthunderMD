#ifndef RT_NATIVE_CAMPAIGN_COMPLETION_SCENE_H
#define RT_NATIVE_CAMPAIGN_COMPLETION_SCENE_H
#include "rt_campaign_scene_background.h"
#include "rt_campaign_scene_art.h"
#include "rt_campaign_transition.h"
typedef struct {s32 x,y;u8 kind,visible,layer;} RT_CompletionGeometry;
typedef struct {
 RT_CompletionGeometry highGeometry[RT_NATIVE_STAGE_CAPACITY];
 u8 highCached,highPopulation,highCount;
 RT_StagePool highActors;
 RT_SceneCast lowActors;
 RT_TransitionClock clock;
 RT_SceneArtRuntime lowArt;
 RT_SpriteBatch batch;
 u32 scratch[225],highRowBudget[224];
 const RT_TransitionArt *highArt;
 const RT_SceneBackground *background;
 u32 firstRefresh,lastRefresh;
 u16 frames,latePublications,publicationLine;
 volatile u8 visible,enableOnVInt,pending,fault;
} RT_CompletionScene;
#ifdef __m68k__
/* Exclusive video owner. Preserves/restores SDK VInt and VBlank callbacks;
 * gameplay video must be reloaded after End. Does not service audio callbacks.
 * No stage advancement or bonus ownership; caller delivers transition events.
 * Begin/Step failure requires End before any other VDP owner is installed. */
u8 RT_completionSceneBegin(RT_CompletionScene *scene,u8 selector);
/* Final mode12 uses the same renderer but never runs the ordinary countdown.
 * Step keeps returning1 until the caller observes actual sound completion and
 * calls End. This API does not synthesize a sound acknowledgement. */
u8 RT_completionEndingBegin(RT_CompletionScene *scene,u8 selector);
/* Bounded initialization with paired caller-owned bus-window callbacks. */
u8 RT_completionEndingBeginIO(RT_CompletionScene *scene,u8 selector,
 void (*beginIO)(void),void (*endIO)(void));
/* One native actor update and acknowledged presentation:0fault,1running,2done.
 * First frame enables only at real VInt. Later transfers publish before joy
 * polling; clock never advances ahead of accepted rendering. */
u8 RT_completionSceneStep(RT_CompletionScene *scene);
/* Optional exclusive frame-I/O owner (both callbacks or neither). Begin waits
 * for its verified bus window; end releases it. With callbacks, SDK VBlank
 * tasks run IMMEDIATELY in that window. First-frame setup is bracketed too.
 * uploadTiles is the exact contiguous staged low-pattern DMA count (0..160),
 * so the owner can budget its window for this frame's actual transfer.
 * This API does not establish or validate a PCM-safe schedule itself. */
u8 RT_completionSceneStepIO(RT_CompletionScene *scene,
 void (*beginIO)(u8 uploadTiles),void (*endIO)(void));
/* Prepare Home credits outside IO; next Step publishes within its owned window. */
u8 RT_completionSceneSetCredits(RT_CompletionScene*,u16);
void RT_completionSceneEnd(RT_CompletionScene *scene);
#endif
#endif
