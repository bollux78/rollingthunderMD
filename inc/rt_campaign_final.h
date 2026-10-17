#ifndef RT_NATIVE_CAMPAIGN_FINAL_H
#define RT_NATIVE_CAMPAIGN_FINAL_H
#include "rt_campaign_score.h"
/* Native mode12 phases4..8. Begin follows the real final-monitor/music09
 * admission; the earlier victory sound gate remains the caller's responsibility.
 * This owns sequencing only, never VDP/audio devices or an emulated DP image. */
typedef struct {
 RT_CampaignBonus bonus;
 u32 refresh;
 u8 phase,player;
} RT_CampaignFinal;
enum {RT_FINAL_NATIVE_BONUS=4,RT_FINAL_NATIVE_WAIT=5,RT_FINAL_NATIVE_LOAD=6,
 RT_FINAL_NATIVE_SCENE=7,RT_FINAL_NATIVE_CLEAR=8,RT_FINAL_NATIVE_ENTRY=9};
typedef enum {RT_FINAL_NATIVE_INVALID,RT_FINAL_NATIVE_RUNNING,
 RT_FINAL_NATIVE_ADVANCED,RT_FINAL_NATIVE_HIGH_SCORE} RT_CampaignFinalResult;
enum {RT_FINAL_NATIVE_SAVE_PLAYER=1,RT_FINAL_NATIVE_LOAD_SCENE=2,
 RT_FINAL_NATIVE_CLEAR_SCENE=4,RT_FINAL_NATIVE_CREDITS=8};
typedef struct {u8 bonusEvents,actions;} RT_CampaignFinalEvents;
u8 RT_campaignFinalBegin(RT_CampaignFinal *final,u16 timer,u8 player,u32 refresh);
/* Source phase7 requires BOTH acknowledged scene presentation and available,
 * zero sound status. Missing status is never silence. PCM drain is not this
 * gate. Caller applies returned actions exactly once before the next Step:
 * SAVE_PLAYER clears player weapons/lives HUD, saves profile and clears panel;
 * LOAD_SCENE writes credits/clears health-ammo glyphs and constructs scene;
 * CREDITS publishes current credits before other scene actions (also while
 * an acknowledged scene waits for sound);
 * CLEAR_SCENE clears panel and resets high actors before high-score entry.
 * An ADVANCED result may be serviced again in the same physical refresh. */
RT_CampaignFinalResult RT_campaignFinalStep(RT_CampaignFinal *final,u32 refresh,
 RT_CampaignScore *score,u8 setting,u8 *reserves,u8 scenePresented,
 u8 soundStatusAvailable,u8 soundStatus,RT_CampaignFinalEvents *events);
/* Phase5 inventory/life fields (source C0/CA..CE). Clear the completing
 * agent and its native restart inventory, retaining pose, location, health,
 * score and selection. Pass Arcade reserves; pass NULL for Home, whose shared
 * purchased credits are not a player reserve and must survive this cleanup.
 * HUD publication/profile metadata and panel clearing remain caller-owned. */
u8 RT_campaignFinalClearInventory(RT_Agent *agent,RT_Agent *restartAgent,
 u8 *arcadeReserves);
#endif
