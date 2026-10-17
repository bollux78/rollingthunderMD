#ifndef RT_NATIVE_CAMPAIGN_LOOP_H
#define RT_NATIVE_CAMPAIGN_LOOP_H
#include "rt_campaign_scene.h"
typedef enum {
    RT_CAMPAIGN_RUNNING,RT_CAMPAIGN_CONTINUE,RT_CAMPAIGN_GAME_OVER,
    RT_CAMPAIGN_ARCADE_DEATH,RT_CAMPAIGN_FAULT,RT_CAMPAIGN_STAGE_CLEAR,
    RT_CAMPAIGN_FINAL_EXIT
} RT_CampaignRunState;
typedef struct {
    RT_CampaignPlayerPass players;
    RT_CampaignBattlePass battle;
    RT_CampaignObjectPass objects;
    u8 respawnMask,updated,faultPhase,worldUpdated;
    u8 exitSounds[2]; /* After ordinary player sounds on the latch update. */
} RT_CampaignUpdate;
/* Elapsed physical refreshes, not simulation steps or newly drawn frames.
 * Zero is valid for a catch-up step; saturation handles pauses cheaply. */
void RT_campaignProtectionAdvance(RT_CampaignScene *scene,u32 refreshes);
/* One ordered gameplay update. Lifecycle states do not advance the world.
 * Home survivor respawns consume shared credits; both-dead Continue is handed
 * to the application, which must restore scene/camera before resuming.
 * Arcade death is separate: no Home respawn/credit policy is applied to it.
 * STAGE_CLEAR hands ownership to completion at the source exit threshold;
 * it does not load a new area or bypass bonus/transition presentation.
 * FINAL_EXIT hands off the latched boss gate; it does not certify that sound
 * has drained or the ending has run. The ending owner must perform that wait.
 * Audio, scoring, drawing and stage completion consume these results outside
 * this function. worldUpdated publishes the ordinary world/score phase;
 * updated also includes scripted exit movement, which bypasses that phase.
 * Only entries below each published event count are valid;
 * unused event storage is retained. On a lifecycle return without updated,
 * phase counters other than event counts must not be consumed.
 * Nonzero unservicedKinds still indicates incomplete content. */
RT_CampaignRunState RT_campaignUpdate(RT_CampaignScene *scene,const u16 pads[2],
    u8 tick,u8 refresh,RT_CampaignUpdate *result);
#endif
