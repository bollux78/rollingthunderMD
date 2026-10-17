#ifndef RT_NATIVE_ENEMY_ATLAS_H
#define RT_NATIVE_ENEMY_ATLAS_H
#include "rt_sprites.h"
/* Uncompiled kinds/colors/poses explicitly return null. */
const RT_SpritePose *RT_enemyPose(u8 kind,u8 colorByte,u16 pose);
const RT_PackedSpritePose *RT_enemyGraphic(u8 kind,u8 colorByte,u16 pose);
#endif
