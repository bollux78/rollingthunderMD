#include "rt_type28.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type28ChildStep(RT_Enemy *e,const RT_Type28MotionContext *c)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || e->state.kind!=40 ||
       (e->state.action&0xfc)!=0x6c || e->state.row>11)return RT_ENEMY_UNAVAILABLE;
    u8 row=e->state.row;
    if(row<=6){
        u8 code;
        RT_EnemySweep result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        if(result==RT_ENEMY_SWEEP_HIT){
            e->state.action^=3;e->state.spawnAction=e->state.action;
            e->velocityX=(s16)(0-(u16)e->velocityX);
        }
        u16 scratch=0;
        result=RT_highEnemyVertical(e,c->camera,*c->entry.accelerationY,c->shadowYLow,c->probe,c->probeContext,&scratch);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        if(result==RT_ENEMY_SWEEP_HIT)e->state.row=7;
        else{
            if(--e->state.timer)return RT_ENEMY_CONTINUE;
            e->state.row=row==6?4:row+1;
        }
    }else{
        if(--e->state.timer)return RT_ENEMY_CONTINUE;
        if(row==11)return RT_type28Select(e,0x28,&c->entry)==RT_TYPE28_ENTRY_INSTALLED?
            RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
        e->state.row++;
    }
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[8]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
