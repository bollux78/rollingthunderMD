#include "rt_type28.h"
RT_EnemyResult RT_type28Step(RT_Enemy *e,const RT_Type28MotionContext *c,
    u8 population,RT_Type28Spawn spawn,void *spawnContext)
{
    if(!e || !c || e->state.kind!=40 || e->state.action>=0xc0)return RT_ENEMY_UNAVAILABLE;
    switch(e->state.action&0xfc){
    case 0x18:case 0x1c:case 0x64:case 0x84:case 0x88:case 0x94:case 0xa0:case 0xa4:
        return RT_type28SimpleStep(e,&c->entry);
    case 0x28:return RT_type28ArcStep(e,c);
    case 0x30:return RT_type28SweepStep(e,c);
    case 0x48:return RT_ENEMY_CONTINUE; /* C3A2 RTS, not C39B entry reversal. */
    case 0x60:case 0x68:case 0x78:case 0x7c:case 0x98:
        return RT_type28ParentStep(e,c,population,spawn,spawnContext);
    case 0x6c:return RT_type28ChildStep(e,c);
    case 0x80:return RT_type28Startup(e,&c->entry)==RT_TYPE28_ENTRY_INSTALLED?
        RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
    default:return RT_type28WalkStep(e,c);
    }
}
