#include "rt_type24.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type24LaunchStep(RT_Enemy *e,const RT_Type24MotionContext *c)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || !c->entry.command ||
       !c->entry.sound || e->state.kind!=36 || e->state.row>5)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc,row=e->state.row;
    if(state!=0x20 && state!=0x24 && state!=0x78 && state!=0x7c)return RT_ENEMY_UNAVAILABLE;
    if(row>=4){
        u8 code;
        RT_EnemySweep result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    }
    if(row>=2){
        u16 scratch=0;
        RT_EnemySweep result=RT_highEnemyVertical(e,c->camera,*c->entry.accelerationY,
            c->shadowYLow,c->probe,c->probeContext,&scratch);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    }
    if(row==2){
        s16 sourceY=(s16)(((s32)c->camera->y+256)*16-e->state.y);
        if(sourceY<0x900)return RT_ENEMY_CONTINUE;
        e->velocityY=-64;*c->entry.accelerationY=8;
    }else{
        if(--e->state.timer)return RT_ENEMY_CONTINUE;
        if(row==1){if(--e->delay)e->state.row=255;}
        else if(row==3){*c->entry.accelerationY=4;e->velocityX=e->state.action&2?-16:16;}
        else if(row==4){*c->entry.accelerationY=0;e->velocityX=e->state.action&2?-32:32;}
        else if(row==5){
            *c->entry.sound=0x62;
            return RT_type24Select(e,4,&c->entry)==RT_TYPE24_ENTRY_INSTALLED?
                RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
        }
    }
    e->state.row++;
    /* E1AA follows the four E19A idle rows in compiled sequence1. */
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[1]+4+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
