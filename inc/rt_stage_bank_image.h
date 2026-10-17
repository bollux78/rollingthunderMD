#include "rt_stage_bank.h"
#include "rt_stage_bank_image_v2.h"
typedef RT_StageBankImageV2 RT_StageBankImage;
#define RT_STAGE_BANK_MAGIC RT_STAGE_BANK_MAGIC_V2
#define RT_STAGE_BANK_IMAGE ((const RT_StageBankImage*)RT_STAGE_BANK_BASE)
