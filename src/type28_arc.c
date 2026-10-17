#include "rt_type28.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type28ArcStep(RT_Enemy *e,const RT_Type28MotionContext *c)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || e->state.kind!=40 ||
       (e->state.action&0xfc)!=0x28 || e->state.row>2)return RT_ENEMY_UNAVAILABLE;
    u8 code;
    RT_EnemySweep result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    s16 x=(s16)(e->state.x-(s32)c->camera->x*16);
    if(result==RT_ENEMY_SWEEP_HIT || ((e->state.action&2)?x<0x100:x>=0x1100)){
        e->state.action^=3;e->state.spawnAction=e->state.action;
        e->velocityX=(s16)(0-(u16)e->velocityX);
    }
    u16 scratch=0;
    result=RT_highEnemyVertical(e,c->camera,*c->entry.accelerationY,c->shadowYLow,c->probe,c->probeContext,&scratch);
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    if(e->state.row==2){
        s16 sourceY=(s16)(((s32)c->camera->y+256)*16-e->state.y);
        if(sourceY>=0x1000){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        return RT_ENEMY_CONTINUE;
    }
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    e->state.row++;
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[10]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
