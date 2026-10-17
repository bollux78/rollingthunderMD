#include "rt_type24.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type24WalkStep(RT_Enemy *e,const RT_Type24MotionContext *c)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || !c->entry.command ||
       !c->entry.sound || e->state.kind!=36 || e->state.row>5)return RT_ENEMY_UNAVAILABLE;
    switch(e->state.action&0xfc){
    case 4:case 8:case 0x0c:case 0x10:case 0x14:case 0x28:case 0x2c:
    case 0x30:case 0x34:case 0x38:case 0x3c:case 0x40:case 0x4c:case 0x50:
    case 0x58:case 0x5c:case 0x70:case 0x74:case 0x8c:case 0x90:case 0x9c:
    case 0xa8:case 0xac:case 0xb0:case 0xb4:case 0xb8:case 0xbc:break;
    default:return RT_ENEMY_UNAVAILABLE;
    }
    u8 code,next=0;
    RT_EnemySweep result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    if(result==RT_ENEMY_SWEEP_HIT)next=0x48;
    else{
        if(!e->velocityY){
            if(e->delay)e->delay--;
            else if(!(e->state.decisions&0x10) &&
                    ((e->velocityX<0)!=!!(e->state.decisions&4)))next=0x44;
        }
        s16 x=(s16)(e->state.x-(s32)c->camera->x*16);
        if(!next && (e->velocityX<0?x<0x200:x>0x1000))next=0x48;
    }
    if(next)return RT_type24Select(e,next,&c->entry)==RT_TYPE24_ENTRY_INSTALLED?
        RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
    RT_EnemyResult motion=RT_type24Oscillate(e,c);
    if(motion!=RT_ENEMY_CONTINUE)return motion;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    u8 row=e->state.row;
    if(row==2 || row==5)*c->entry.command=8;
    e->state.row=row==5?0:row+1;
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[15]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
