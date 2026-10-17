#ifndef RT_NATIVE_SURVIVAL_CHECKPOINT_H
#define RT_NATIVE_SURVIVAL_CHECKPOINT_H
#include "rt_survival_scene.h"
/* Capture a supported, upright live player's location for party Continue.
 * Advances only forwards, retaining each player's fresh entry template.
 * Current-position solo death and survivor respawn remain Home policy.
 * Returns1 on capture,0 if no suitable standing location is available.
 * stage.entrances[1] records its camera; retirement rebases it atomically. */
u8 RT_survivalCheckpointCapture(RT_CampaignScene *scene,RT_SurvivalWorld *world);
#endif
