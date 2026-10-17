#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyApproachEntries[4]={
    {8,0x53,0x6efc},{8,0x53,0x6f09},{8,0x53,0x6f1c},{8,0x53,0x6f2f}
};
RT_EnemyResult RT_enemyApproachStep(RT_Enemy *e,s32 playerX,u8 *selector)
{
    if(!e || !selector)return RT_ENEMY_UNAVAILABLE;
    u8 kind=e->state.kind&0x7f,state=e->state.action&0xfc,row=e->state.row;
    if((kind!=4 && kind!=8 && kind!=12) ||
       (state!=0xb0 && state!=0xb8 && state!=0xbc))return RT_ENEMY_UNAVAILABLE;
    /* CD88 checks the hit latch before validating the animation row. */
    if(e->byte12==255){e->byte12=0;*selector=0x44;return RT_ENEMY_SELECT;}
    if(row>3)return RT_ENEMY_UNAVAILABLE;
    *selector=0;
    s16 distance=(s16)((u16)e->state.x-(u16)playerX);
    if(!row){if(distance>=0xc00)return RT_ENEMY_CONTINUE;}
    else if(row==3){
        if(distance<0x600){*selector=0x44;return RT_ENEMY_SELECT;}
        return RT_ENEMY_CONTINUE;
    }else if(--e->state.timer)return RT_ENEMY_CONTINUE;
    const RT_EnemyEntry *entry=rtEnemyApproachEntries+row+1;
    e->state.row=row+1;e->state.timer=entry->timer;e->state.flags=entry->flags;
    e->state.pose=284+row+1;
    return RT_ENEMY_CONTINUE;
}
