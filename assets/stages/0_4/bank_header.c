#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area04.h"
extern const u16 rtStageWorld04Cells[][256],rtStageWorld04Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea04Graphics,area04_rtDisplayDoorPatches,rtStageWorld04Cells,rtStageWorld04Directory,43,0,4};
