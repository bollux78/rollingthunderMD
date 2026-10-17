#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area13.h"
extern const u16 rtStageWorld13Cells[][256],rtStageWorld13Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea13Graphics,area13_rtDisplayDoorPatches,rtStageWorld13Cells,rtStageWorld13Directory,62,1,3};
