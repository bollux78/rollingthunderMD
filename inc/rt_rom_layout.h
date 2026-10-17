#ifndef RT_ROM_LAYOUT_H
#define RT_ROM_LAYOUT_H
/* Default frozen layout. The compact builder generates a private replacement. */
#define RT_ROM_BYTES 0x2000000UL
#define RT_CART_EXTENDED_SSF 0
#define RT_FINAL_VICTORY_WINDOWS 1
#define RT_COMPLETION_PAGE 50
#define RT_HIGH_SCORE_PAGE 54
#define RT_HIGH_SCORE_BASE 0x100000UL
#define RT_HIGH_COLD_ADDRESS (55UL*524288)
#define RT_HIGH_WARM_ADDRESS (58UL*524288)
static const u32 rtCampaignBankOffsets[10]={0,0,0,0,0,0,0,0,0,0};
#endif
