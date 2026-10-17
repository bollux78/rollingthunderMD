#ifndef RT_NATIVE_SURVIVAL_RETIRE_H
#define RT_NATIVE_SURVIVAL_RETIRE_H
#include "rt_survival_display.h"
#include "rt_survival_rebase_native.h"
#include "rt_campaign_display.h"
/* Try to retire two sections at a post-presentation boundary. Returns1 only
 * after world, generator, scene and display have all moved. Returns0 without
 * mutation if consumers still retain the old sections or inputs disagree.
 * pendingObjects must have been consumed (count0), including audio effects.
 * This does not invent a safe Continue checkpoint: the owner must refresh
 * home.spawn positions before their old sections can be retired. */
u8 RT_survivalRetireTwo(RT_SurvivalWindow *window,RT_SurvivalWorld *world,
    RT_CampaignScene *scene,RT_CampaignDisplay *view,RT_SurvivalDisplay *display,
    const RT_CampaignObjectPass *pendingObjects,const RT_WorldStage *source);
/* Read-only spatial preflight before waiting for an in-flight presentation.
 * Does not certify DMA/display ownership; RetireTwo checks that after drain. */
u8 RT_survivalRetireSpatiallyReady(const RT_CampaignScene *scene);
/* After the initial entry, retain one complete local section behind camera.
 * The generated stage installs this bound for native camera admission. */
u16 RT_survivalLocalCameraMinimum(const RT_SurvivalWindow *window);
#endif
