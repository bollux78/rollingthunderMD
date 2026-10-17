#ifndef area04_RT_HIGH_ENEMY_GRAPHICS_H
#define area04_RT_HIGH_ENEMY_GRAPHICS_H
#include "rt_sprites.h"
/* Family32 normal20/death30 family36 color40, family40 color32 and family44 color26; unsupported values return null. */
const RT_PackedSpritePose *area04_RT_highEnemyGraphic(u8 kind,u8 color,u16 pose);
#endif
