#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area12.h"
extern const u16 rtStageWorld12Cells[][256],rtStageWorld12Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea12Graphics,area12_rtDisplayDoorPatches,rtStageWorld12Cells,rtStageWorld12Directory,66,1,2};
