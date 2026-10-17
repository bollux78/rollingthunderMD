#ifndef RT_NATIVE_CAMPAIGN_ENDING_H
#define RT_NATIVE_CAMPAIGN_ENDING_H
#include "rt_campaign_final.h"
#include "rt_campaign_completion_scene.h"
#include "rt_campaign_completion_monitor.h"
/* Zero-initialize before first Begin. Exclusive native ending owner. Caller drains gameplay before Begin, admits
 * the matching source track09 after Begin, and supplies REAL sequencer status.
 * IO callbacks must own the verified PCM-safe window. No guessed audio timer,
 * stage-bank remap, music admission, or high-score UI is hidden here.
 * Caller publishes CREDITS actions and applies earnedReserves to Home shared
 * credits after each Step; Arcade reserves are updated directly. */
typedef struct {
 RT_CampaignFinal flow;
 RT_CampaignFinalEvents events;
 RT_CompletionScene *scene;
 RT_CampaignScore *score;
 RT_Agent *agent,*restart;
 u8 *arcadeReserves;
 u8 active,sceneLoaded,entryReady,fault,earnedReserves;
} RT_CampaignEnding;
typedef enum {RT_ENDING_FAULT,RT_ENDING_RUNNING,RT_ENDING_ENTRY_READY} RT_EndingResult;
#ifdef __m68k__
u8 RT_campaignEndingBegin(RT_CampaignEnding *ending,RT_CompletionScene *scene,
 RT_CampaignScore *score,RT_Agent *agent,RT_Agent *restart,u8 *arcadeReserves,
 u8 player,u16 timer);
RT_EndingResult RT_campaignEndingStep(RT_CampaignEnding *ending,u32 refresh,
 u8 soundStatusAvailable,u8 soundStatus,void (*beginFrame)(u8),
 void (*beginLoad)(void),void (*endIO)(void));
/* Always call before another video owner, including on failure. Music remains
 * caller-owned; source status0 and PCM drained are deliberately distinct. */
void RT_campaignEndingEnd(RT_CampaignEnding *ending);
#endif
#endif
