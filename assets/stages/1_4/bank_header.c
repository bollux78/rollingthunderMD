#include "rt_stage_bank_image_v2.h"
#include "rt_campaign_area14.h"
extern const u16 rtStageWorld14Cells[][256],rtStageWorld14Directory[];
__attribute__((section(".stage.header"),used))
const RT_StageBankImageV2 rtStageBankImage={RT_STAGE_BANK_MAGIC_V2,&rtCampaignArea14Graphics,area14_rtDisplayDoorPatches,rtStageWorld14Cells,rtStageWorld14Directory,51,1,4};
