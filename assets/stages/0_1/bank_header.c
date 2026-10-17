#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area01.h"
extern const u16 rtStageWorld01Cells[][256],rtStageWorld01Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea01Graphics,area01_rtDisplayDoorPatches,rtStageWorld01Cells,rtStageWorld01Directory,63,0,1};
