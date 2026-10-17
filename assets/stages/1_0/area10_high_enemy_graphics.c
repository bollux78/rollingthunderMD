#include "area10_rt_high_enemy_graphics.h"
#include "area10_rt_high10_poses.h"
#include "area10_rt_high15_poses.h"
const RT_PackedSpritePose *area10_RT_highEnemyGraphic(u8 kind,u8 color,u16 pose)
{
    const RT_PackedSpritePose *p=0;
    if(pose<383)return 0;
    if(kind==32 && color==20 && pose<area10_RT_HIGH10_POSE_COUNT)p=area10_rtHigh10PosesGraphics+pose;
    if(kind==32 && color==30 && pose<area10_RT_HIGH15_POSE_COUNT)p=area10_rtHigh15PosesGraphics+pose;
    return p && p->pose?p:0;
}
