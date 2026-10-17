#ifndef RT_NATIVE_DISPLAY_RUNTIME_H
#define RT_NATIVE_DISPLAY_RUNTIME_H
#include "rt_display.h"
typedef struct {
    const RT_DisplayStage *stage;
    s16 column,row;
    s16 scrollX,scrollY;
    u16 queuedBytes;
    u8 ready;
} RT_DisplayRuntime;
/* Initialization owns the VDP while blanked. All stage terrain patterns stay
 * resident. HUD/entity pattern allocation must start above patternCount. */
/* Shared VDP setup for resident and streamed backends. Leaves display blank
 * and ready=0; backend must upload its initial cells before enabling video. */
u8 RT_displayConfigure(RT_DisplayRuntime *display,const RT_DisplayStage *stage,
    const RT_WorldCamera *camera);
u8 RT_displayInitialize(RT_DisplayRuntime *display,const RT_DisplayStage *stage,
    const RT_WorldCamera *camera);
/* Queue only newly exposed rows/columns. Ordinary camera movement is bounded
 * to two cells per axis per update (matching the 16px terrain window bound).
 * Larger scene changes need initialization.
 * The caller flushes DMA in VBlank, then commits the corresponding scroll. */
u8 RT_displayPrepare(RT_DisplayRuntime *display,const RT_WorldCamera *camera);
/* Queue a compiled plane-A rectangle after prepare, before the VBlank flush.
 * World cell coordinates are clipped to the current resident window. Cells
 * must remain immutable until the queued DMA finishes (normally ROM data).
 * This is publication only: scene lifetime and repaint after streamed terrain
 * overwrites are the caller's responsibility. No collision data is changed.
 * A queue failure invalidates the display; reinitialize rather than commit a
 * partly queued frame. Successful off-window patches intentionally do no work. */
u8 RT_displayPatch(RT_DisplayRuntime *display,const RT_DisplayPatch *patch,
    s16 worldColumn,s16 worldRow);
void RT_displayCommit(const RT_DisplayRuntime *display);
#endif
