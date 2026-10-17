#include "rt_type2c.h"
RT_EnemyResult RT_type2cWallResponse(RT_Enemy *e,u8 code,const RT_Type2cEntryContext *c)
{
    if(!e || !c || e->state.kind!=44)return RT_ENEMY_UNAVAILABLE;
    e->byte12|=1;
    u8 state=e->state.action&0xfc,selector;
    if(state!=4 && state!=0x40)return RT_ENEMY_CONTINUE;
    switch(code){
    case 0x19:case 0x22:selector=0x48;break;
    case 0x25:case 0x27:selector=0x28;break;
    case 0x2d:selector=0x2c;break;
    default:selector=0x24;break;
    }
    RT_Type2cEntryResult result=RT_type2cSelect(e,selector,c);
    if(result==RT_TYPE2C_ENTRY_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_TYPE2C_ENTRY_DECLINED)
        return RT_type2cSelect(e,0x48,c)==RT_TYPE2C_ENTRY_INSTALLED?
            RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
    return RT_ENEMY_CONTINUE;
}
