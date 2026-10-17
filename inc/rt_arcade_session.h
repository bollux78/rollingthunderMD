#ifndef RT_NATIVE_ARCADE_SESSION_H
#define RT_NATIVE_ARCADE_SESSION_H
#include "rt_campaign_scene.h"
typedef struct {
    u8 reserves,checkpoint,story,area;
    RT_EventWindow previousWindow;
} RT_ArcadeSession;
/* Source 9CB6/A131:3 or5 total lives; first entry consumes one. Native decimal
 * reserves replace packed-BCD profile bytes. No Home shared-credit policy. */
u8 RT_arcadeSessionInitialize(RT_ArcadeSession *session,const RT_CampaignScene *scene,u8 livesSetting);
/* Source A6C2 checkpoint condition at an event-grid boundary. */
void RT_arcadeObserveWindow(RT_ArcadeSession *session,const RT_CampaignScene *scene);
/* Called once per completed life. Zero means game over, without underflow.
 * Caller reconstructs scene at checkpoint and runs the source128-refresh ready
 * wait. Score, stage transitions and continue purchase are separate owners. */
u8 RT_arcadeClaimRestart(RT_ArcadeSession *session);
#endif
