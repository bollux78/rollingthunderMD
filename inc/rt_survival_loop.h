#ifndef RT_NATIVE_SURVIVAL_LOOP_H
#define RT_NATIVE_SURVIVAL_LOOP_H
#include "rt_survival_scene.h"
#include "rt_campaign_loop.h"
/* Native generated-world update. Publishes the same ordered sound, score and
 * object event batches as Campaign, but never runs its countdown or exit gate.
 * Continue/game-over returns freeze simulation; the application owns Continue
 * input, safe restart placement and publication. Physical-refresh protection
 * remains RT_campaignProtectionAdvance, independent of skipped draws.
 * Generated-world bounds apply in native motion; owner retires chunks after publication.
 * Density and walking-guard decision pressure follow the generated tier;
 * other enemy families retain their original timing. */
RT_CampaignRunState RT_survivalUpdate(RT_CampaignScene *scene,
    RT_SurvivalWorld *world,const u16 pads[2],u8 tick,u8 refresh,
    RT_CampaignUpdate *result);
#endif
