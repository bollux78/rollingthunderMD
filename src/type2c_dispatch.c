#include "rt_type2c.h"
RT_EnemyResult RT_type2cStep(RT_Enemy *e,const RT_Type2cMotionContext *c,
    const RT_StagePool *stage,s32 targetX)
{
    if(!e || !c || e->state.kind!=44)return RT_ENEMY_UNAVAILABLE;
    switch(e->state.action&0xfc){
    case 0:case 4:case 8:case 0x0c:case 0x10:case 0x14:case 0x3c:
    case 0x40:case 0x4c:case 0x50:case 0x58:case 0x5c:case 0x8c:
    case 0x90:case 0x9c:case 0xa8:case 0xb0:case 0xb4:case 0xb8:case 0xbc:{
        RT_Type2cWalkContext walk={.entry=c->entry,.stage=stage,
            .targetX=targetX,.shadowXLow=c->shadowXLow};
        return RT_type2cWalkStep(e,&walk);
    }
    case 0x18:case 0x1c:case 0x38:case 0x48:case 0x54:case 0x70:
    case 0x74:case 0x78:case 0x7c:case 0x94:case 0x98:
        return RT_type2cSimpleStep(e,&c->entry);
    case 0x80:
        return RT_type2cStartup(e,&c->entry)==RT_TYPE2C_ENTRY_UNAVAILABLE?
            RT_ENEMY_UNAVAILABLE:RT_ENEMY_CONTINUE;
    case 0x20:case 0x24:case 0x28:case 0x2c:case 0x30:case 0x34:
    case 0x44:case 0x60:case 0x64:case 0x68:case 0x6c:case 0x84:
    case 0x88:case 0xa0:case 0xa4:case 0xac:
        return RT_type2cMotionStep(e,c);
    default:return RT_ENEMY_UNAVAILABLE;
    }
}
