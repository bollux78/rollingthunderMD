#ifndef RT_NATIVE_CAMPAIGN_SCENE_ART_H
#define RT_NATIVE_CAMPAIGN_SCENE_ART_H
#include "rt_campaign_scene_actor.h"
#include "rt_campaign_transition_art.h"
#define RT_SCENE_ART_TILES 160
#define RT_SCENE_ART_CHUNKS 32
typedef struct {s16 dx[2],dy;u16 tile;u8 size,palette,flipX,flipY;} RT_SceneArtPart;
typedef struct {u16 first;u8 count,reserved;} RT_SceneArtPose;
typedef struct {
 const u32 *patterns;const u16 *palette;const RT_SceneArtPart *parts;
 const RT_SceneArtPose *poses[3];u16 poseCount,partCount,tileCount;
} RT_SceneArt;
extern const RT_SceneArt rtSceneArt;
typedef struct {u16 tile,count,destination;u8 left,right;} RT_SceneArtChunk;
typedef struct {
 u32 pixels[RT_SCENE_ART_TILES*8];
 RT_SceneArtChunk chunks[RT_SCENE_ART_CHUNKS];
 RT_SpriteBatch *preparedBatch;const RT_SceneArt *boundArt;
 u16 firstTile,capacity,used,chunkCount,dirtyFirst,dirtyEnd;
 u8 prepared,dirty;
} RT_SceneArtRuntime;
u8 RT_sceneArtInitialize(RT_SceneArtRuntime *runtime,u16 firstTile,u16 capacity);
/* Append low actors to the prepared high SAT. Reuses unchanged staged pixels,
 * deduplicates component chunks, masks viewport edges, checks combined H40
 * limits. Failure invalidates preparation; no hardware or actor state changes.
 * Caller installs the lower-region palette/raster split before publication. */
u8 RT_sceneArtPrepare(RT_SceneArtRuntime *runtime,const RT_SceneArt *art,
 const RT_SceneCast *cast,const RT_TransitionArt *highArt,
 RT_SpriteBatch *batch,u32 scratch[225]);
/* Optional verified high-prefix occupancy (count<<16 | width) for each row.
 * Caller must retain the exact validated high SAT until this call. NULL uses
 * the full combined validation path. Only low-occupied rows need rechecking. */
u8 RT_sceneArtPrepareBudget(RT_SceneArtRuntime *runtime,const RT_SceneArt *art,
 const RT_SceneCast *cast,const RT_TransitionArt *highArt,
 RT_SpriteBatch *batch,u32 scratch[225],const u32 highRowBudget[224]);
#ifdef __m68k__
u8 RT_sceneArtPublish(RT_SceneArtRuntime *runtime,RT_SpriteBatch *batch);
#endif
#endif
