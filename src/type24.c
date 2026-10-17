#include "rt_type24.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type24SimpleStep(RT_Enemy *e,const RT_Type24EntryContext *c)
{
    if(!e || !c || e->state.kind!=36 || e->state.row>3)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc,row=e->state.row;
    u8 death=state>=0x60 && state<=0x6c;
    if(!death && state!=0 && state!=0x98)return RT_ENEMY_UNAVAILABLE;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    const RT_HighAnimationRow *r;
    if(death){
        if(row==3){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        e->state.row++;
        /* E1EA is the final four rows of the compiled E1DA sequence. */
        r=rtHighCycleRows+rtHighCycleOffsets[17]+4+e->state.row;
    }else{
        if(row==0 || row==2){e->state.action^=3;e->state.spawnAction=e->state.action;}
        else{
            u8 flags=e->state.decisions;
            if(!e->delay && ((flags&0x10) || ((e->state.action&2)?!(flags&4):(flags&4))))
                return RT_type24Select(e,0x20,c)==RT_TYPE24_ENTRY_INSTALLED?
                    RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
        }
        if(row==3){e->delay--;e->state.row=0;}else e->state.row++;
        r=rtHighCycleRows+rtHighCycleOffsets[1]+e->state.row;
    }
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
