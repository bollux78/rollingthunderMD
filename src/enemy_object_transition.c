#include "rt_enemies.h"
/* CPU2 DCBC/DCDC: native pose IDs268..283 retain the two source lists. */
const RT_EnemyEntry rtEnemyObjectEntries[16]={
    {5,0x24,0x631d},
    {5,0x23,0x6330},
    {5,0x00,0x6343},
    {5,0xf6,0x6356},
    {5,0xf6,0x637c},
    {5,0xf6,0x638f},
    {5,0xf6,0x63b5},
    {5,0xf6,0x63c8},
    {5,0x81,0x61da},
    {5,0x81,0x6460},
    {12,0xf6,0x63db},
    {6,0xf6,0x6401},
    {6,0xf6,0x6414},
    {6,0x00,0x6427},
    {5,0x24,0x643a},
    {5,0x25,0x644d},
};
RT_EnemyResult RT_enemyObjectTransitionStep(RT_Enemy *e,u8 *selector)
{
    if(!e || !selector)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc,row=e->state.row;
    if((state!=0xa8 && state!=0xac) || row>7)return RT_ENEMY_UNAVAILABLE;
    *selector=0;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    if(row==7){
        *selector=state==0xa8 || (e->state.decisions&1)?4:0x38;
        return RT_ENEMY_SELECT;
    }
    if(row>=2 && row<=5){
        /* Source Y is upward-positive; native world Y is downward-positive. */
        e->state.y+=state==0xa8?-32:32;
        if(state==0xa8 && row==5)e->state.attributes=0x40;
        if(state==0xac && row==2)e->state.attributes=0x80;
        if(state==0xac && row==5){
            if(!(e->state.decisions&0x20)){*selector=8;return RT_ENEMY_SELECT;}
            e->state.action=e->state.spawnAction=e->state.decisions&4?0xad:0xae;
        }
    }
    u8 index=(state==0xac?8:0)+row+1;
    const RT_EnemyEntry *entry=rtEnemyObjectEntries+index;
    e->state.row=row+1;e->state.timer=entry->timer;e->state.flags=entry->flags;
    e->state.pose=268+index;
    return RT_ENEMY_CONTINUE;
}
