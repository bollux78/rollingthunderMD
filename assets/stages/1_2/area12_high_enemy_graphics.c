#include "area12_rt_high_enemy_graphics.h"
#include "area12_rt_high10_poses.h"
#include "area12_rt_high15_poses.h"
#include "area12_rt_high13_poses.h"
#include "area12_rt_high20_poses.h"
#include "area12_rt_high16_poses.h"
const RT_PackedSpritePose *area12_RT_highEnemyGraphic(u8 kind,u8 color,u16 pose)
{
    const RT_PackedSpritePose *p=0;
    if(pose<383)return 0;
    if(kind==32 && color==20 && pose<area12_RT_HIGH10_POSE_COUNT)p=area12_rtHigh10PosesGraphics+pose;
    if(kind==32 && color==30 && pose<area12_RT_HIGH15_POSE_COUNT)p=area12_rtHigh15PosesGraphics+pose;
    if(kind==36 && color==40 && pose<area12_RT_HIGH20_POSE_COUNT)p=area12_rtHigh20PosesGraphics+pose;
    if(kind==40 && color==32 && pose<area12_RT_HIGH16_POSE_COUNT)p=area12_rtHigh16PosesGraphics+pose;
    if(kind==44 && color==26 && pose<area12_RT_HIGH13_POSE_COUNT)p=area12_rtHigh13PosesGraphics+pose;
    return p && p->pose?p:0;
}
