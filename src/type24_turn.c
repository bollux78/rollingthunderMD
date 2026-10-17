#include "rt_type24.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type24Oscillate(RT_Enemy *e,const RT_Type24MotionContext *c)
{
    u16 scratch=0;
    RT_EnemySweep result=RT_highEnemyVertical(e,c->camera,*c->entry.accelerationY,
        c->shadowYLow,c->probe,c->probeContext,&scratch);
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    /* CA51 uses equality, not crossing: retain signed16 magnitude at -32768. */
    u16 magnitude=e->velocityY<0?(u16)(0-(u16)e->velocityY):(u16)e->velocityY;
    if(magnitude==40+((e->variation>>4)&3)*8)
        *c->entry.accelerationY=(s16)(0-(u16)*c->entry.accelerationY);
    return RT_ENEMY_CONTINUE;
}
RT_EnemyResult RT_type24TurnStep(RT_Enemy *e,const RT_Type24MotionContext *c)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || e->state.kind!=36 ||
       e->state.row>4 || ((e->state.action&0xfc)!=0x48 && (e->state.action&0xfc)!=0x54))
        return RT_ENEMY_UNAVAILABLE;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    u8 row=e->state.row;
    if(row>=3){
        u8 code;
        RT_EnemySweep result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    }
    RT_EnemyResult motion=RT_type24Oscillate(e,c);
    if(motion!=RT_ENEMY_CONTINUE)return motion;
    if(row==2){
        e->state.action^=3;e->state.spawnAction=e->state.action;
        e->velocityX=(s16)(0-(u16)e->velocityX);
    }
    const RT_HighAnimationRow *r;
    if(row==4){
        /* CAB2 enters through B5AE directly, bypassing C968 entry effects. */
        e->state.action=e->state.spawnAction=(e->state.action&3)|4;
        e->state.row=0;r=rtHighStateRows[1]+1;
    }else{
        e->state.row++;r=rtHighCycleRows+rtHighCycleOffsets[16]+e->state.row;
    }
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
