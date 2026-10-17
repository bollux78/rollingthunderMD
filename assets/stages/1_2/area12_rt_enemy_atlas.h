#ifndef area12_RT_NATIVE_ENEMY_ATLAS_H
#define area12_RT_NATIVE_ENEMY_ATLAS_H
#include "rt_sprites.h"
/* Uncompiled kinds/colors/poses explicitly return null. */
const RT_SpritePose *area12_RT_enemyPose(u8 kind,u8 colorByte,u16 pose);
const RT_PackedSpritePose *area12_RT_enemyGraphic(u8 kind,u8 colorByte,u16 pose);
#endif
