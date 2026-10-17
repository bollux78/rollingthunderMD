#include "rt_type10.h"
static u8 enter(RT_Enemy *e,u8 selector,u8 phase,u8 step,
    RT_Type10Effects *effects,RT_Type10PassEvent *event)
{
    if(!(selector&3))selector+=e->state.action&3;
    e->state.spawnAction=selector;
    u8 state=selector&0xfc;
    if(!state && phase==6 && step==7)effects->sharedCommand=event->command=8;
    if(state==0x60 || state==0x64 || state==0x68){
        if(!RT_type10Hit(e,selector,effects))return 0;
        event->sound=effects->sound;
        if(!e->byte6)event->command=7;
        return 1;
    }
    return RT_type10Enter(e,selector);
}
/* Rare per-actor service: keep it out of the hot pool traversal. */
__attribute__((noinline))
RT_EnemyResult RT_type10UpdateWithDelay(RT_Enemy *e,const RT_WorldCamera *camera,
    const RT_Agent *target,u8 phase,u8 step,RT_Type10Effects *effects,
    RT_Type10PassEvent *event,u8 *admissionDelay)
{
    if(!e || !camera || !target || !effects || !event || e->state.kind!=16)return RT_ENEMY_UNAVAILABLE;
    *event=(RT_Type10PassEvent){0};
    RT_DoorEnemy *a=&e->state;u8 state=a->action&0xfc,pending=a->spawnAction&0xfc;
    u8 uncounted=state==0x80 && pending!=0x5c && pending!=0x8c;
    s16 x=(s16)(a->x-(s32)camera->x*16),y=(s16)(((s32)camera->y+256)*16-a->y);
    u8 inside;
    if(uncounted){
        if(y>=-0x400 && y<0x1000 && (a->spawnAction&2?x>=0x1500:x< -0x300)){
            if(!enter(e,a->spawnAction,phase,step,effects,event))return RT_ENEMY_UNAVAILABLE;
        }
        inside=!a->hidden;state=a->action&0xfc;
    }else inside=y>=-0x400 && y<0x1000 && x>=-0x300 && (state==0x80?x<0x1500:x<=0x1500);
    if(!inside){a->hidden=1;return RT_ENEMY_CONTINUE;}
    /* Home reinforcements wait only after source grid activation. In the
     * uncounted state80 branch, hidden is retained by the source; suppressing
     * that branch would strand an actor before its outgoing-boundary entry. */
    if(!uncounted && admissionDelay && *admissionDelay){
        (*admissionDelay)--;a->hidden=1;return RT_ENEMY_CONTINUE;
    }
    a->hidden=0;event->visibleDelta=!uncounted;
    if(!uncounted && a->spawnAction!=a->action){
        if(!enter(e,a->spawnAction,phase,step,effects,event))return RT_ENEMY_UNAVAILABLE;
        state=a->action&0xfc;
    }
    RT_enemyPerceiveReady(e,target);
    if(state>0x80)return RT_ENEMY_UNAVAILABLE;
    u8 selector;RT_EnemyResult result=RT_type10Step(e,camera,target->body.x,effects,&selector);
    if(result==RT_ENEMY_REMOVED){event->visibleDelta--;event->deaths=state>=0x60 && state<=0x6c;}
    else if(result==RT_ENEMY_SELECT && !enter(e,selector,phase,step,effects,event))return RT_ENEMY_UNAVAILABLE;
    return result;
}

RT_EnemyResult RT_type10Update(RT_Enemy *e,const RT_WorldCamera *camera,
    const RT_Agent *target,u8 phase,u8 step,RT_Type10Effects *effects,
    RT_Type10PassEvent *event)
{
    return RT_type10UpdateWithDelay(e,camera,target,phase,step,effects,event,0);
}
