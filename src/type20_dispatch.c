#include "rt_type20.h"
/* Dispatch for all48 masked family-table slots. Scene work
 * (visibility, pending hits, perception, counts) remains the caller's job. */
RT_EnemyResult RT_type20Step(RT_Enemy *e,const RT_Type20Motion *c)
{
    if(!e)return RT_ENEMY_UNAVAILABLE;
    switch(e->state.action&0xfc){
    case 0x38:case 0x48:case 0x54:case 0x60:case 0x64:case 0x68:case 0x6c:
        return RT_type20SimpleStep(e);
    case 0x18:case 0x1c:case 0x20:case 0x24:case 0x28:case 0x2c:
    case 0x30:case 0x34:case 0x44:case 0x94:case 0xa0:case 0xa4:
        return RT_type20MotionStep(e,c);
    default:return RT_type20WalkStep(e,c);
    }
}
