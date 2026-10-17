#include "rt_type24.h"
RT_EnemyResult RT_type24Step(RT_Enemy *e,const RT_Type24MotionContext *c)
{
    if(!e || !c || e->state.kind!=36)return RT_ENEMY_UNAVAILABLE;
    switch(e->state.action&0xfc){
    case 0:case 0x98:case 0x60:case 0x64:case 0x68:case 0x6c:
        return RT_type24SimpleStep(e,&c->entry);
    case 0x48:case 0x54:return RT_type24TurnStep(e,c);
    case 0x20:case 0x24:case 0x78:case 0x7c:return RT_type24LaunchStep(e,c);
    case 0x18:case 0x1c:case 0x44:case 0x84:case 0x88:case 0x94:case 0xa0:case 0xa4:
        return RT_type24ChaseStep(e,c);
    case 0x80:return RT_type24Startup(e,&c->entry)==RT_TYPE24_ENTRY_INSTALLED?
        RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
    default:return RT_type24WalkStep(e,c);
    }
}
