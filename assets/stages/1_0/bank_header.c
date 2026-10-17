#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area10.h"
extern const u16 rtStageWorld10Cells[][256],rtStageWorld10Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea10Graphics,area10_rtDisplayDoorPatches,rtStageWorld10Cells,rtStageWorld10Directory,48,1,0};
