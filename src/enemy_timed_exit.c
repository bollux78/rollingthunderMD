#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyTimedExitEntries[19]={
{4,0x1a,0x6dc1},
{4,0x1a,0x6da8},
{4,0x1a,0x6d89},
{4,0x1a,0x6d6a},
{4,0x1a,0x6d4b},
{4,0x1a,0x6d32},
{4,0xf2,0x6d1f},
{4,0xf2,0x6d0c},
{4,0xf2,0x6cf9},
{4,0xf2,0x6ce6},
{4,0xf2,0x6cd3},
{4,0xf2,0x6cc0},
{4,0xf2,0x6cad},
{4,0xf2,0x6ca0},
{8,0x01,0x6085},
{8,0x01,0x6000},
{8,0x01,0x6b52},
{8,0x01,0x6000},
{8,0x01,0x6b52},
};
RT_EnemyResult RT_enemyTimedExitStep(RT_Enemy *e,u8 *selector)
{
    if(!e || !selector)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc,row=e->state.row;
    if(state!=0x90 && state!=0x98)return RT_ENEMY_UNAVAILABLE;
    u8 last=state==0x90?13:4;
    if(row>last)return RT_ENEMY_UNAVAILABLE;
    *selector=0;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    if(row==last){
        if(state==0x90){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        *selector=0x94;return RT_ENEMY_SELECT;
    }
    u8 index=(state==0x90?0:14)+row+1;
    const RT_EnemyEntry *entry=rtEnemyTimedExitEntries+index;
    e->state.row=row+1;e->state.timer=entry->timer;e->state.flags=entry->flags;
    e->state.pose=288+index;return RT_ENEMY_CONTINUE;
}
