#ifndef RT_NATIVE_CAMPAIGN_HUD_DISPLAY_H
#define RT_NATIVE_CAMPAIGN_HUD_DISPLAY_H
#include "rt_campaign_hud_art.h"
typedef struct {
    const RT_CampaignHudArt *art;
    RT_CampaignHud previous;
    u16 firstTile;
    u8 ready;
} RT_CampaignHudDisplay;
/* Exclusive HInt owner, preserving/chaining the prior VInt callback. Requires
 * display disabled, empty DMA and a reserved pattern range below planeA C000.
 * Window usesD000; normal64x32 A/B scrolling remains unchanged. */
u8 RT_campaignHudDisplayBegin(RT_CampaignHudDisplay *display,
    const RT_CampaignHudArt *art,u16 firstTile);
/* Queue only changed cell spans; copied DMA buffers may leave the stack. */
u8 RT_campaignHudDisplayQueue(RT_CampaignHudDisplay *display,const RT_CampaignHud *hud);
/* Bit per logical row. Caller accumulates dirty rows across skipped draws and
 * clears them only after a successful queue. */
u8 RT_campaignHudDisplayQueueRows(RT_CampaignHudDisplay *display,const RT_CampaignHud *hud,u8 dirtyRows);
/* End before menus, stage remapping, or another raster owner. */
void RT_campaignHudDisplayEnd(RT_CampaignHudDisplay *display);
#endif
