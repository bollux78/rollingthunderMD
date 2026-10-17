#include "area01_rt_high_enemy_graphics.h"
const RT_PackedSpritePose *area01_RT_highEnemyGraphic(u8 kind,u8 color,u16 pose)
{
    (void)kind;(void)color;
    const RT_PackedSpritePose *p=0;
    if(pose<383)return 0;
    return p && p->pose?p:0;
}
