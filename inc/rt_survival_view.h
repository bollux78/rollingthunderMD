#ifndef RT_NATIVE_SURVIVAL_VIEW_H
#define RT_NATIVE_SURVIVAL_VIEW_H
#include "rt_survival_display.h"
#include "rt_campaign_display.h"
/* Generated terrain plus ordinary native door patches in one publication
 * owner. The Level1 atlas remains resident. Both owners need stable storage. */
u8 RT_survivalViewInitialize(RT_CampaignDisplay *view,RT_SurvivalDisplay *stream,
    const RT_CampaignScene *scene,const RT_SurvivalWorld *world,
    const RT_DisplayStage *atlas,const RT_DisplayPatch patches[3][2]);
/* Queue every simulation update, including skipped draws, before consuming
 * the object event batch. Terrain precedes door writes. Call the existing
 * RT_campaignDisplayCommitted only after DMA/scroll/SAT publication; then
 * coordinated retirement may rebase these same owners. Never use Campaign's
 * authored-map Queue with this view. Failure after queuing invalidates ready. */
u8 RT_survivalViewQueue(RT_CampaignDisplay *view,RT_SurvivalDisplay *stream,
    const RT_WorldCamera *camera,const RT_CampaignObjectPass *objects);
#endif
