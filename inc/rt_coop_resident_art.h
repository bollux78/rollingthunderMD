#ifndef RT_COOP_RESIDENT_ART_H
#define RT_COOP_RESIDENT_ART_H
#include "rt_campaign_sprites.h"
#define RT_COOP_RESIDENT_TILES 256
extern const u32 *const rtCoopPatterns[10];
extern const RT_PackedSpritePose rtCoopArt[10][10];
extern const u16 rtCoopOffsets[10][10];
u8 RT_coopResidentBegin(RT_CampaignSprites*,u8 players);
u8 RT_coopResidentIndex(const RT_Enemy*);
const RT_PackedSpritePose *RT_coopResidentGraphic(u8 index);
RT_SpriteSlot *RT_coopResidentSlot(u8 index);
/* Presentation-only policy: gameplay and animation clocks are untouched. */
extern volatile u8 rtCoopCompact;
void RT_coopDetailUpdate(u8 players,u8 enemies,u8 projectiles,u8 late);
void RT_coopDetailPressure(void);
#endif
