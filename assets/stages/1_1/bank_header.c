#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area11.h"
extern const u16 rtStageWorld11Cells[][256],rtStageWorld11Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea11Graphics,area11_rtDisplayDoorPatches,rtStageWorld11Cells,rtStageWorld11Directory,63,1,1};
