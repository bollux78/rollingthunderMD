#include "rt_enemy_templates.h"
const RT_EnemyTemplate *RT_doorEnemyTemplate(u8 stage,u8 worldXLow,u8 counter)
{
    if(stage>=10)return 0;
    u8 choice=((u8)(worldXLow+counter)&0x1c)>>2;
    return rtDoorEnemyTemplates+rtDoorEnemyTemplateIds[stage][choice];
}
