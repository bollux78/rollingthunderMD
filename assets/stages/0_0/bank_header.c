#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area00.h"
extern const u16 rtStageWorld00Cells[][256],rtStageWorld00Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea00Graphics,area00_rtDisplayDoorPatches,rtStageWorld00Cells,rtStageWorld00Directory,47,0,0};
