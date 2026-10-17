#ifndef RT_NATIVE_CAMPAIGN_EXIT_H
#define RT_NATIVE_CAMPAIGN_EXIT_H
#include "rt_campaign_scene.h"
#include "rt_campaign_progress.h"
/* After ordinary player service, before enemy/projectile/world service.
 * An ordinary exit latches once; Home carries every living partner through
 * the same scripted walk. Dead slots stay dead and spend no credits here.
 * Final completion needs a real boss-complete input, never a screen shortcut.
 * FINAL latches bossProgress=0x80 and exitPlayer; no ordinary walk begins.
 * Audio drain and final presentation remain the ending owner's responsibility. */
RT_CampaignExitResult RT_campaignExitCheck(RT_CampaignScene *scene,u8 bossComplete,u8 sounds[2]);
/* Exclusive player service while exiting. Camera and world remain frozen.
 * Returns0 invalid,1 walking,2 all surviving participants at the handoff. */
u8 RT_campaignExitAdvance(RT_CampaignScene *scene,u8 sounds[2]);
u8 RT_campaignExitComplete(const RT_CampaignScene *scene);
#endif
