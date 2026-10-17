#ifndef RT_NATIVE_CAMPAIGN_DISPLAY_H
#define RT_NATIVE_CAMPAIGN_DISPLAY_H
#include "rt_campaign_scene.h"
#include "rt_display_stream.h"
typedef struct {
    RT_DisplayRuntime display;
    const RT_DisplayPatch (*doorPatches)[2];
    u16 pendingBytes;
    RT_DisplayStream *stream;
} RT_CampaignDisplay;
/* Atlas and patch cells must be immutable and share the selected stage's tile
 * allocation. Reject a mismatched atlas instead of displaying the opening map
 * for another stage. Initializes VDP; caller owns scene/menu transitions. */
u8 RT_campaignDisplayInitialize(RT_CampaignDisplay *view,
    const RT_CampaignScene *scene,const RT_DisplayStage *atlas,
    const RT_DisplayPatch patches[3][2]);
/* Optional streamed backend. Caller owns storage for the scene lifetime;
 * slots is the terrain VRAM reservation used by sprite allocation. */
u8 RT_campaignDisplayInitializeStreamed(RT_CampaignDisplay *view,
    const RT_CampaignScene *scene,const RT_DisplayStage *atlas,
    const RT_DisplayPatch patches[3][2],RT_DisplayStream *stream,u16 slots);
/* Call every simulation update, including skipped presentations, in world
 * update order. Stream terrain before this update's door writes. All queue
 * data is copied or immutable; it remains valid after events are reused.
 * Does not flush DMA or change hardware scroll. No permanent patch overlay:
 * later authored streaming can overwrite an earlier door edit. */
u8 RT_campaignDisplayQueue(RT_CampaignDisplay *view,
    const RT_WorldCamera *camera,const RT_CampaignObjectPass *objects);
/* Call only after the caller has flushed the full graphics queue in VBlank
 * and committed scroll/SAT. A skipped presentation must not call this. */
void RT_campaignDisplayCommitted(RT_CampaignDisplay *view);
#endif
