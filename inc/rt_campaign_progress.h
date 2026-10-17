#ifndef RT_NATIVE_CAMPAIGN_PROGRESS_H
#define RT_NATIVE_CAMPAIGN_PROGRESS_H
#include "rt_agent.h"
typedef struct {u8 column,row;} RT_CampaignExitCell;
extern const RT_CampaignExitCell rtCampaignExitCells[10];
typedef enum {RT_CAMPAIGN_EXIT_INVALID,RT_CAMPAIGN_EXIT_NONE,
    RT_CAMPAIGN_EXIT_WALK,RT_CAMPAIGN_EXIT_FINAL} RT_CampaignExitResult;
/* CPU2 98F0/9A62 gate, expressed in native values. localX uses four fractional
 * bits. ground is the (0,+1) foot probe in downward-positive world space;
 * it is required only inside the ordinary exit's collision-tested interval.
 * A caller owns the once-only exit latch and final-boss completion flag. */
RT_CampaignExitResult RT_campaignExitGate(u8 story,u8 area,u8 windowX,u8 windowY,
    s16 localX,u8 exiting,u8 bossComplete,u8 layer,const RT_TerrainMaterial *ground);
/* Ordinary area progression, no wrapping after the second story. The final
 * boss takes a separate ending route, never an ordinary area11 load. */
u8 RT_campaignNextArea(u8 story,u8 area,u8 *nextStory,u8 *nextArea);
/* Dedicated scripted walk after the exit latch. Caller suspends ordinary
 * controller movement while this owns the agent. Returns0 for invalid state,
 * 1 while walking,2 at the source296px handoff point. */
u8 RT_campaignExitWalkBegin(RT_Agent *agent,u8 story,u8 area,u8 *sound);
u8 RT_campaignExitWalkStep(RT_Agent *agent,const RT_WorldCamera *camera,
    u8 story,u8 area,u8 *sound);
#endif
