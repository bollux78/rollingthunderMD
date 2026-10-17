#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area03.h"
extern const u16 rtStageWorld03Cells[][256],rtStageWorld03Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea03Graphics,area03_rtDisplayDoorPatches,rtStageWorld03Cells,rtStageWorld03Directory,41,0,3};
