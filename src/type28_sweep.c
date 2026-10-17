#include "rt_type28.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type28SweepStep(RT_Enemy *e,const RT_Type28MotionContext *c)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || e->state.kind!=40 ||
       (e->state.action&0xfc)!=0x30 || e->state.row>2)return RT_ENEMY_UNAVAILABLE;
    u8 code;
    RT_EnemySweep result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    s16 x=(s16)(e->state.x-(s32)c->camera->x*16);
    if(result==RT_ENEMY_SWEEP_HIT || (e->velocityX<0?x<0:x>=0x1200)){
        e->state.action^=3;e->state.spawnAction=e->state.action;
        e->velocityX=(s16)(0-(u16)e->velocityX);
    }
    u16 scratch=0;
    result=RT_highEnemyVertical(e,c->camera,*c->entry.accelerationY,c->shadowYLow,c->probe,c->probeContext,&scratch);
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    if(result==RT_ENEMY_SWEEP_HIT)return RT_type28Select(e,0x28,&c->entry)==RT_TYPE28_ENTRY_INSTALLED?
        RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
    u16 magnitude=e->velocityY<0?(u16)(0-(u16)e->velocityY):(u16)e->velocityY;
    if(magnitude==48)*c->entry.accelerationY=(s16)(0-(u16)*c->entry.accelerationY);
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    e->state.row=e->state.row==2?0:e->state.row+1;
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[11]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
