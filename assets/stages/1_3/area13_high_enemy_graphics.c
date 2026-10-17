#include "area13_rt_high_enemy_graphics.h"
#include "area13_rt_high20_poses.h"
#include "area13_rt_high16_poses.h"
const RT_PackedSpritePose *area13_RT_highEnemyGraphic(u8 kind,u8 color,u16 pose)
{
    const RT_PackedSpritePose *p=0;
    if(pose<383)return 0;
    if(kind==36 && color==40 && pose<area13_RT_HIGH20_POSE_COUNT)p=area13_rtHigh20PosesGraphics+pose;
    if(kind==40 && color==32 && pose<area13_RT_HIGH16_POSE_COUNT)p=area13_rtHigh16PosesGraphics+pose;
    return p && p->pose?p:0;
}
