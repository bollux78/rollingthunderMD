#ifndef RT_NATIVE_TERRAIN_WINDOW_H
#define RT_NATIVE_TERRAIN_WINDOW_H
#include "rt_world.h"
/* Native collision materials, not arcade tile words. Retains the published
 * 44-column footprint and stale/unfilled cells needed by offscreen actors. */
typedef struct {u16 materials[2048];RT_WorldCamera camera;u8 ready;} RT_TerrainWindow;
u8 RT_terrainWindowInitialize(RT_TerrainWindow *window,const RT_WorldStage *stage,
    const RT_WorldCamera *camera);
/* Synchronous publication, horizontal then vertical; each axis <=16px.
 * Separate calls preserve a caller's different axis/service ordering. */
u8 RT_terrainWindowMove(RT_TerrainWindow *window,const RT_WorldStage *stage,
    const RT_WorldCamera *camera);
const RT_TerrainMaterial *RT_terrainWindowProbe(const RT_TerrainWindow *window,
    s32 x,s32 y,s16 cellX,s16 cellY);
#endif
