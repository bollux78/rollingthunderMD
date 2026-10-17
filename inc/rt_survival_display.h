#ifndef RT_NATIVE_SURVIVAL_DISPLAY_H
#define RT_NATIVE_SURVIVAL_DISPLAY_H
#include "rt_survival_world.h"
#include "rt_display_runtime.h"
typedef struct {
    RT_DisplayRuntime *display;
    const RT_SurvivalWorld *world;
    u32 firstChunk;
} RT_SurvivalDisplay;
/* Resident Level1 atlas, using the Campaign's existing 640-tile reservation.
 * Door patches still use RT_displayPatch in local world coordinates. */
u8 RT_survivalDisplayInitialize(RT_SurvivalDisplay *stream,RT_DisplayRuntime *display,
    const RT_DisplayStage *source,const RT_SurvivalWorld *world,const RT_WorldCamera *camera);
u8 RT_survivalDisplayPrepare(RT_SurvivalDisplay *stream,const RT_WorldCamera *camera);
/* After all queued DMA completes and the owner retires exactly two chunks,
 * subtract512px from the display coordinates. The 64-cell plane ring retains
 * the same physical addresses; camera/entities must be rebased by the owner
 * as part of this same operation. No VDP rewrite or forced blank is needed. */
u8 RT_survivalDisplayRebase(RT_SurvivalDisplay *stream);
#endif
