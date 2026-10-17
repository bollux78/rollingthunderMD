#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area02.h"
extern const u16 rtStageWorld02Cells[][256],rtStageWorld02Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea02Graphics,area02_rtDisplayDoorPatches,rtStageWorld02Cells,rtStageWorld02Directory,32,0,2};
