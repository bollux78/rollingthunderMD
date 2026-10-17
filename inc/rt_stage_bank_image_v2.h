#ifndef RT_STAGE_BANK_IMAGE_V2_H
#define RT_STAGE_BANK_IMAGE_V2_H
#include "rt_campaign_sprites.h"
#define RT_STAGE_BANK_MAGIC_V2 0x52544232UL
/* Experimental readonly ABI. Map and validate before terrain initialization.
 * Every pointer expires when the stage bank is replaced. Material definitions
 * remain fixed; directory entries index worldCells within this image only. */
typedef struct {
    u32 magic;
    const RT_CampaignGraphics *graphics;
    const RT_DisplayPatch (*patches)[2];
    const u16 (*worldCells)[256];
    const u16 *worldDirectory;
    u16 worldPageCount;
    u8 story,area;
} RT_StageBankImageV2;
#ifdef __m68k__
_Static_assert(sizeof(RT_StageBankImageV2)==24,"stage v2 ABI");
#endif
#endif
