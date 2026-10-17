#ifndef RT_NATIVE_DISPLAY_STREAM_H
#define RT_NATIVE_DISPLAY_STREAM_H
#include "rt_display_runtime.h"
#include "rt_tile_cache.h"
typedef struct {RT_TileCache tiles;u16 cells[4096];u8 resident;} RT_DisplayStream;
/* If the whole stage atlas fits, load it once and use direct resident maps.
 * Otherwise cache owns tiles0..slots-1; sprite allocation starts at slots. Storage must
 * survive all queued writes. Uses the same map coordinates/VDP layout as the
 * resident backend; commit through RT_displayCommit after a complete flush. */
u8 RT_displayStreamInitialize(RT_DisplayStream *stream,RT_DisplayRuntime *display,
    const RT_DisplayStage *stage,const RT_WorldCamera *camera,u16 slots);
u8 RT_displayStreamPrepare(RT_DisplayStream *stream,RT_DisplayRuntime *display,
    const RT_WorldCamera *camera);
u8 RT_displayStreamPatch(RT_DisplayStream *stream,RT_DisplayRuntime *display,
    const RT_DisplayPatch *patch,s16 column,s16 row);
#endif
