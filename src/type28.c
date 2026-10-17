#include "rt_type28.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type28SimpleStep(RT_Enemy *e,const RT_Type28EntryContext *c)
{
    if(!e || !c || e->state.kind!=40 || e->state.row>3)return RT_ENEMY_UNAVAILABLE;
    u8 death=(e->state.action&0xfc)==0x64;
    switch(e->state.action&0xfc){
    case 0x64:case 0x18:case 0x1c:case 0x84:case 0x88:case 0x94:case 0xa0:case 0xa4:break;
    default:return RT_ENEMY_UNAVAILABLE;
    }
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    if(death && e->state.row==3){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    if(!death && e->state.row==3)return RT_type28Select(e,0x30,c)==RT_TYPE28_ENTRY_INSTALLED?
        RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
    e->state.row++;
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[death?12:3]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
