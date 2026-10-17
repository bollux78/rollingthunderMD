#include "rt_type28.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type28WalkStep(RT_Enemy *e,const RT_Type28MotionContext *c)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || e->state.kind!=40 || e->state.row>5)
        return RT_ENEMY_UNAVAILABLE;
    switch(e->state.action&0xfc){
    case 0:case 4:case 8:case 0x0c:case 0x10:case 0x14:case 0x20:case 0x24:
    case 0x2c:case 0x34:case 0x38:case 0x3c:case 0x40:case 0x44:case 0x4c:
    case 0x50:case 0x54:case 0x58:case 0x5c:case 0x70:case 0x74:case 0x8c:
    case 0x90:case 0x9c:case 0xa8:case 0xac:case 0xb0:case 0xb4:case 0xb8:case 0xbc:break;
    default:return RT_ENEMY_UNAVAILABLE;
    }
    u8 code;
    RT_EnemySweep result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    s16 x=(s16)(e->state.x-(s32)c->camera->x*16);
    if(result==RT_ENEMY_SWEEP_HIT || (e->velocityX<0?x<0x100:x>=0x1100)){
        e->state.action^=3;e->state.spawnAction=e->state.action;
        e->velocityX=(s16)(0-(u16)e->velocityX);
    }
    u16 scratch=0;
    result=RT_highEnemyVertical(e,c->camera,*c->entry.accelerationY,c->shadowYLow,c->probe,c->probeContext,&scratch);
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    static const s16 thresholds[5]={48,24,0,-24,-48};
    s16 speed=(s16)((u16)(0-(u16)e->velocityY)&0xfff8);
    if(result==RT_ENEMY_SWEEP_HIT || (e->state.row==5 && speed==0)){
        e->velocityY=-72;e->state.row=0;
    }else{
        if(e->state.row==5 || speed>thresholds[e->state.row])return RT_ENEMY_CONTINUE;
        e->state.row++;
    }
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[2]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
