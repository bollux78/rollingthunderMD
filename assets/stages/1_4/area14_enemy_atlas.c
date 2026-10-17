/* Compiled door/grid guard variants; no runtime source decoding. */
#include "area14_rt_enemy_atlas.h"
#include "area14_rt_enemy_b0_g2_poses.h"
#include "area14_rt_enemy_b0_g3_poses.h"
#include "area14_rt_enemy_b0_g4_poses.h"
#include "area14_rt_enemy_b0_g40_poses.h"
#include "area14_rt_enemy_b0_g41_poses.h"
#include "area14_rt_enemy_b0_g42_poses.h"
#include "area14_rt_enemy_b0_g43_poses.h"
#include "area14_rt_enemy_b1_g6_poses.h"
#include "area14_rt_enemy_b1_g7_poses.h"
#include "area14_rt_enemy_b1_g8_poses.h"
#include "area14_rt_enemy_b1_g9_poses.h"
#include "area14_rt_enemy_b1_g17_poses.h"
#include "area14_rt_enemy_b1_g18_poses.h"
static const RT_PackedSpritePose *const variants[2][128]={
{
[2]=area14_rtEnemy_b0_g2PosesGraphics,
[3]=area14_rtEnemy_b0_g3PosesGraphics,
[4]=area14_rtEnemy_b0_g4PosesGraphics,
[40]=area14_rtEnemy_b0_g40PosesGraphics,
[41]=area14_rtEnemy_b0_g41PosesGraphics,
[42]=area14_rtEnemy_b0_g42PosesGraphics,
[43]=area14_rtEnemy_b0_g43PosesGraphics,
},
{
[6]=area14_rtEnemy_b1_g6PosesGraphics,
[7]=area14_rtEnemy_b1_g7PosesGraphics,
[8]=area14_rtEnemy_b1_g8PosesGraphics,
[9]=area14_rtEnemy_b1_g9PosesGraphics,
[17]=area14_rtEnemy_b1_g17PosesGraphics,
[18]=area14_rtEnemy_b1_g18PosesGraphics,
},
};
const RT_SpritePose *area14_RT_enemyPose(u8 kind,u8 colorByte,u16 pose){
 if((kind!=4 && kind!=8 && kind!=12 && kind!=16) || pose>=383)return 0;
 const RT_PackedSpritePose *list=variants[kind==8 || kind==16][colorByte>>1];
 return list?list[pose].pose:0;
}
const RT_PackedSpritePose *area14_RT_enemyGraphic(u8 kind,u8 colorByte,u16 pose){
 if((kind!=4 && kind!=8 && kind!=12 && kind!=16) || pose>=383)return 0;
 const RT_PackedSpritePose *list=variants[kind==8 || kind==16][colorByte>>1];
 return list && list[pose].pose?list+pose:0;
}
