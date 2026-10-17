#include "rt_type2c.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
static RT_EnemyResult selectState(RT_Enemy *e,u8 selector,const RT_Type2cEntryContext *c)
{
    return RT_type2cSelect(e,selector,c)==RT_TYPE2C_ENTRY_UNAVAILABLE?
        RT_ENEMY_UNAVAILABLE:RT_ENEMY_CONTINUE;
}
RT_EnemyResult RT_type2cWalkStep(RT_Enemy *e,const RT_Type2cWalkContext *c)
{
    if(!e || !c || !c->entry.camera || !c->stage || e->state.kind!=44)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc;
    switch(state){
    case 0:case 4:case 8:case 0x0c:case 0x10:case 0x14:case 0x3c:
    case 0x40:case 0x4c:case 0x50:case 0x58:case 0x5c:case 0x8c:
    case 0x90:case 0x9c:case 0xa8:case 0xb0:case 0xb4:case 0xb8:case 0xbc:break;
    default:return RT_ENEMY_UNAVAILABLE;
    }
    if(e->state.row>5)return RT_ENEMY_UNAVAILABLE;
    const RT_Type2cEntryContext *entry=&c->entry;
    u8 code,selector;
    RT_EnemySweep sweep=RT_highEnemyHorizontal(e,entry->camera,c->shadowXLow,entry->probe,entry->probeContext,&code);
    if(sweep==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(sweep==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    if(sweep==RT_ENEMY_SWEEP_HIT)return RT_type2cWallResponse(e,code,entry);
    if(!RT_highEnemyGround(e,entry->camera,entry->probe,entry->probeContext,&selector))return RT_ENEMY_UNAVAILABLE;
    if(selector)return selectState(e,selector,entry);
    if(!RT_type2cObjectProbe(e,c->stage,entry->camera,entry->phase,&selector))return RT_ENEMY_UNAVAILABLE;
    if(selector)return selectState(e,selector,entry);
    RT_EnemyResult decision=RT_type2cWalkDecision(e,c->targetX,entry);
    if(decision!=RT_ENEMY_CONTINUE)return decision==RT_ENEMY_SELECT?RT_ENEMY_CONTINUE:decision;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    if(e->state.row==5){e->state.row=0;e->byte12&=0xdf;}else e->state.row++;
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[29]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
